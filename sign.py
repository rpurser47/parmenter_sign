import json
import urllib2
import time
import serial

url = "https://www.kimonolabs.com/api/24afmnvi?apikey=j0qCMngQnLkG8EI6t7gRkwrW8KfTaTgN"

maxValue = 380000
minValue = -100000
segments = 8
period = 5

while 1:
    try:
        port = serial.Serial("/dev/ttyAMA0", baudrate = 9600, timeout=3.0)
        jsonThermo = json.loads(urllib2.urlopen(url).read())
        currentLevel = int(jsonThermo['results']['collection1'][0]['current'])

        segmentsToLight = int(round(float(currentLevel - minValue)/float(maxValue - minValue) * segments))

        command = "+"

        for i in range(8):
            if i < segmentsToLight:
                command = command + "9"
            else:
                command = command + "0"

        command = command + "-"

        port.write(command)

        port.close()
    except:
        print("Warning -- had an error: ")
        port.close()
    time.sleep(period)
