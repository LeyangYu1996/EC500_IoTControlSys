The folder contains 4 files except Readme.

Please compile and run PID.cpp on the LED node with the following commands:
g++ PID.cpp -o PID -pthread -lwiringPi
./PID 80800 2000
When running the program, 80800 indicates the port number; 2000 indicates the wanted brightness.
These 2 arguments can be changed.
If you want to change the target brightness value when the program is running, please type in the number directly and press enter.

Please compile and run 
The State.sct is the state machine simulation. Please open it in YAKINDU and run.
Please manully raise the 'dataRead' event to indicate a data is read from the sensor, in order to make the simulation run.
