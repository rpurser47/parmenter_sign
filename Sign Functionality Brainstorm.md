# Sign Functionality Brainstorm
1. ~~Brightness~~
    2. ~~Turn the sign on at 6AM, off at 10PM~~
    2. ~~Run at high brightness 8AM - 5PM~~
    3. ~~Run at medium brightness 6AM - 8AM, 3PM - 5PM~~
    4. ~~Run at low brightness 5PM - 10PM~~
7. Frequency
    1.  On value change or test, Sign starts out getting website data every five seconds
    2.  After 2 minutes, drops to 15 seconds
    3.  After 10 minutes, drops to 1 minute
    4.  After 1 hour, drops to 5 minutes
    5.  After 6 hours, drops to 15 minutes
    6.  _Simple:_ Divide elapsed time by 10, with limit of 5 to 900 seconds.
5. Test button
    6. Press a button, and sign goes to "all on", then updates from website
6.  Reporting back
    7.  Report temperature to ThingSpeak
    8.  Report light level to ThingSpeak
    9.  Report "OK" to thingspeak