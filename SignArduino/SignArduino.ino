
// PWM pins for segments
unsigned int segment1 = 2;
unsigned int segment8 = 9;

// Max value for 100% intensity is 2.5 Volts, or 128 counts.  Minimum is .4 volts, or 21 counts
unsigned int maxOutput = 128;
unsigned int minOutput = 21;

bool fTestMode = false;

void setupLEDs(void)
{
  for(unsigned int iPin = segment1; iPin <= segment8; iPin++)
  {
    pinMode(iPin, OUTPUT);
  }
  
  // POST -- Power On Self Test
  unsigned int phase1[] = {100, 100, 100, 100, 100, 100, 100, 100};
  unsigned int phase2[] = {100,  10, 100,  10, 100,  10, 100,  10};
  unsigned int phase3[] = { 10, 100,  10, 100,  10, 100,  10, 100};
  unsigned int phase4[] = { 10,  10,  10,  10,  10,  10,  10,  10                                                                                                                                                                                                                                                };
  unsigned int phase5[] = {  0,   0,   0,   0,   0,   0,   0,   0};
  
  setLEDs(phase1);
  delay(1000);
  
  setLEDs(phase2);
  delay(1000);
  
  setLEDs(phase3);
  delay(1000);
  
  setLEDs(phase4);
  delay(1000);
  
  setLEDs(phase5);
  delay(1000);
}

void setLEDs(unsigned int * LEDsetting)
{
  for(unsigned int iLED = 0; iLED < 8; iLED++)
  {
    unsigned int value = *(LEDsetting + iLED);
    unsigned int outputValue;
    if(value == 0)
    {
      outputValue = 0;
    }
    else
    {
      outputValue = map(value, 1, 100, minOutput, maxOutput);
    }
    analogWrite(iLED + segment1, outputValue);
  }
}

void setupBaseStationConnection()
{
  Serial.begin(9600);
}

byte mockSerialBuffer[100];
byte * mockCurrentWriteByte = mockSerialBuffer;
byte * mockCurrentReadByte = mockSerialBuffer;

void resetMockBufferIfNeeded()
{
  if(mockCurrentWriteByte == mockCurrentReadByte)
  {
    mockCurrentWriteByte = mockSerialBuffer;
    mockCurrentReadByte = mockSerialBuffer;
  }
}

int mockWriteToSerial(char * message)
{
  unsigned int offset = 0;
  while (*(message + offset) != 0 && offset < 20)
  {
    *(mockCurrentWriteByte + offset) = *(message + offset);
    offset++;
  }    
  *(mockCurrentWriteByte + offset) = 0;
  mockCurrentWriteByte = mockCurrentWriteByte + offset;
  //Serial.print("wrote '");Serial.print((char *)message);Serial.println("' to mock Serial");
}

char testString[][20] = {
  "+90909090-",
  "+09090909-",
  "isafas+99999999-",
  "09090-",
  "+90909090",
  "-+90909090-",
  "+99090-",
  "+94345d99-",
  "+345623477832-",
  "+12345678-",
  "+87654321-",
  "+00000000-"
};
  
int testState = 0;

void driveTestIfNeeded()
{
  if(!fTestMode)
  {
    return;
  }
  mockWriteToSerial(testString[testState]);
  testState = (testState + 1) % (sizeof(testString)/sizeof(testString[0]));
}

int virtPeek()
{
  if(!fTestMode)
  {
    return Serial.peek();
  }
  
  if(mockCurrentWriteByte == mockCurrentReadByte)
  {
    return -1;
  }
  else
  {
    return *mockCurrentReadByte;
  }
}

int virtAvailable()
{
  if(!fTestMode)
  {
    return Serial.available();    
  }
  return mockCurrentWriteByte - mockCurrentReadByte;
}

int virtRead()
{
  if(!fTestMode)
  {
    return Serial.read();
  }
  int result = virtPeek();
  mockCurrentReadByte++;
  resetMockBufferIfNeeded();
  return result;
}

int virtReadBytesUntil(char stopAt, byte * buf, int bufLength)
{
  if(!fTestMode)
  {
    return Serial.readBytesUntil(stopAt, buf, bufLength);
  }
  int result = 0;
  while(*mockCurrentReadByte != 0 && result < bufLength - 1)
  {
    if(*mockCurrentReadByte == stopAt)
    {
      mockCurrentReadByte++;
      break;
    }
    *(buf + result) = *mockCurrentReadByte;
    mockCurrentReadByte++;
    result++;
  }
  *(buf + result) = 0;
  resetMockBufferIfNeeded(); 

  return result;
}

bool checkForBaseStationCommand(unsigned int * command, unsigned int * LEDCommand)
{
  // Packet is a plus sign (43), eight digits 0-9 (48 - 57), and then a minus sign (45)
  if(virtAvailable() < 10)
  {
    //Serial.print("Not enough in buffer. ");Serial.print(virtAvailable());Serial.println(" bytes in buffer.");
    return false;
  }
   
  do
  {
    if(virtPeek() == 43) // '+' character -- start of packet (43)
    {
      // Start of packet detected.
      break;
    }
    else
    {
      char charDiscarded = virtRead(); // Not start of packet -- discard
      //Serial.print("Discarded ");Serial.println(charDiscarded);
    }
  } while(virtAvailable() != 0);  
  
  if(virtAvailable() < 10)
  {
    // Partial valid packet (or no packet) -- leave it in the buffer, and wait until next time
    Serial.print("Partial packet. ");Serial.print(virtAvailable());Serial.println(" bytes in buffer.");
    return false;
  }
  
  byte incomingBuffer[50];
  // Read up to the minus sign (45) -- end of packet
  int numBytes =  virtReadBytesUntil(45,incomingBuffer,50);
  if(numBytes != 9)
  {
    Serial.print("Discarded (wrong length) '");Serial.print((char *)incomingBuffer);Serial.print("' length was ");Serial.println(numBytes);
    return false;
  }
  
  // incoming buffer should have a complete message '+', 8 digits, '-'
  if(incomingBuffer[0] != 43)
  {
    Serial.print("Discarded (no +) ");Serial.println((char *)incomingBuffer);
    return false;
  }
 
  for(unsigned int iDigit = 0; iDigit < 8; iDigit++)
  {
    if(incomingBuffer[iDigit + 1] < 48 || incomingBuffer[iDigit + 1] > 57)
    {
      Serial.print("Discarded (digits bad) ");Serial.println((char *)incomingBuffer);
      return false;
    }
    LEDCommand[iDigit] = (incomingBuffer[iDigit + 1] - 48) * 10;
  }
  incomingBuffer[10] = 0;
  Serial.print("Accepted ");Serial.println((char *)incomingBuffer);
  return true;
}

void setup() {
  // put your setup code here, to run once:
  setupLEDs();
  setupBaseStationConnection();
}

void loop() {
  unsigned int command;
  unsigned int LEDCommand[] = {  0,  0,  0,  0,   0,   0,   0,   0};
  
  driveTestIfNeeded();
  if(checkForBaseStationCommand(&command, LEDCommand))
  {
    setLEDs(LEDCommand);
  }
  delay(100);
 }

