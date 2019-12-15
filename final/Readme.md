The folder contains 4 files except Readme.
PID.cpp is used for LED node when running wirelessly.
Seneor.cpp is used for sensor node when running wirelessly.
Wired.cpp is used for wired system test.
State.sct is a state machine simulation of the system.

To run with wireless system model please follow the following instructions：
Compile and run PID.cpp on the LED node with the following commands:
```
g++ PID.cpp -o PID -pthread -lwiringPi
./PID 80800 2000
```
When running the program, '80800' indicates the port number it occupies from the OS; '2000' indicates the wanted brightness. These 2 arguments can be changed.
If you want to change the target brightness value when the program is running, type in the number directly and press enter.
Compile and run Sensor.cpp on the sensor node with the following commands:
```
g++ Sensor.cpp -o sensor
./sensor 10.0.0.28 80800
```
'10.0.0.28' is the IP address of LED node, '80800' is the port number of PID program as above.

To run with wired system model please follow the following instructions:
Compile and run Wired.cpp on a single Raspberry Pi wth the following commands:
```
g++ Wired.cpp -o wired -pthread -lwiringPi
./wired 2000
```
'2000' is the target brightness and can be changed.

The State.sct is the state machine simulation. Please open it in YAKINDU and run.
Please manully raise the 'dataRead' event to indicate a data is read from the sensor, in order to make the simulation run.
