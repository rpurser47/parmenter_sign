import json
import urllib2
import time
import serial

try:
    port = serial.Serial("/dev/ttyAMA0", baudrate = 9600, timeout=3.0)
    port.write("+90000000-")
    time.sleep(0.4)
    port.write("+09000000-")
    time.sleep(0.4)
    port.write("+00900000-")
    time.sleep(0.4)
    port.write("+00090000-")
    time.sleep(0.4)
    port.write("+00009000-")
    time.sleep(0.4)
    port.write("+00000900-")
    time.sleep(0.4)
    port.write("+00000090-")
    time.sleep(0.4)
    port.write("+00000009-")
    time.sleep(0.4)
    port.write("+11111111-")
    port.close()
except:
    print("Warning -- had an error.")

url = "https://www.kimonolabs.com/api/24afmnvi?apikey=j0qCMngQnLkG8EI6t7gRkwrW8KfTaTgN"

maxValue = 380000
minValue = -100000
segments = 8
period = 5
previousLevel = -1

while 1:
    try:
        jsonThermo = json.loads(urllib2.urlopen(url).read())

        currentLevel = int(jsonThermo['results']['collection1'][0]['current'])

        if currentLevel != previousLevel:
            valueChangedAt = time.clock()
            previousLevel = currentLevel

        period = (time.clock() - valueChangedAt) / 10
        if period < 5:
            period = 5
        elif period > 900:
            period = 900

        segmentsToLight = int(round(float(currentLevel - minValue)/float(maxValue - minValue) * segments))

        timeofday = time.localtime().tm_hour
        if timeofday < 6 or timeofday >= 22:
            segmentcommand = "0"
        elif (timeofday >= 6 and timeofday < 8) or (timeofday >= 15 and timeofday < 17):
            segmentcommand = "4"
        elif timeofday >= 17 and timeofday < 22:
            segmentcommand = "1"
        else:
            segmentcommand = "9"

        command = "+"
        for i in range(8):
            if i < segmentsToLight:
                command = command + segmentcommand
            else:
                command = command + "0"

        command = command + "-"

        port = serial.Serial("/dev/ttyAMA0", baudrate = 9600, timeout=3.0)
        port.write(command)
        port.close()
    except:
        print("Warning -- had an error.")

    time.sleep(period)
