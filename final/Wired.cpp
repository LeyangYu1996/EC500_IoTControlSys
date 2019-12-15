/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>       /* time_t, struct tm, difftime, time, mktime */
#include <chrono>
#include <thread>
#include <sstream>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <errno.h>
#include <cstring>
#include <sys/ioctl.h>
#include <wiringPi.h>
#include <pthread.h>
#include <fstream>
#include <iostream>

#define I2C_ADDR 0x23

using namespace std::chrono;

int u = 0;
float target = 0;

float read_func();

void errormsg(const char *msg)
{
    perror(msg);
    exit(1);
}

void* PWM(void* args)
{
	while(1)
	{
		digitalWrite(1, LOW);
		digitalWrite(4, LOW);
		delayMicroseconds(1000-u);
		digitalWrite(1, HIGH);
		digitalWrite(4, HIGH);
		delayMicroseconds(u);
	}
}

void* TAR(void* args)
{
     while(1)
	  std::cin>>target;
}

int main(int argc, char *argv[])
{
     wiringPiSetup();
     pinMode(1, OUTPUT);
     pinMode(4, OUTPUT);
     
     float kp=0.1;
     float ki=0.2;
     float kd=0;
     float error;
     float actual=0;
     float integral = 0;
     float pre_error = 0;
     
     target = std::stof(argv[1]);
     std::ofstream myfile;
     myfile.open ("model.txt");
     
     // Start PWM thread.
     pthread_t  threadPWM;
     int ret1;
     
     ret1 = pthread_create(&threadPWM, NULL, PWM, NULL);
     
     if(ret1){
	  fprintf(stderr, "ERROR: Thread create returns :%d\n", ret1);
	  exit(EXIT_FAILURE);
     }
	 
     pthread_t  threadTAR;
     int ret2;
     
     ret2 = pthread_create(&threadTAR, NULL, TAR, NULL);
     
     if(ret2){
	  fprintf(stderr, "ERROR: Thread create returns :%d\n", ret2);
	  exit(EXIT_FAILURE);
     }

        long long int current_time;


      milliseconds ms = duration_cast< milliseconds >(
           system_clock::now().time_since_epoch());

      long long int t4 = ms.count();

     while(1){
	  ms = duration_cast< milliseconds >(
	       system_clock::now().time_since_epoch());

	  current_time = ms.count() - t4;
	  
	  
	  actual = read_func();
	  
	  myfile<<current_time<<"	"<<actual<<"	"<<u<<std::endl;
         
	  printf("Actual value: %f\n", actual);;
         
	  error = target-actual;
	  integral += error;
	  u=kp*error + ki*integral + kd*(error-pre_error);
	  pre_error = error;
	  if (u>1000)
	  {
	       u = 1000;
	       printf("Reach maximun!\n");
	  }
	  else if (u<0) u=0;
	  printf("Adjustment PWM u=%d\n", u);
	  //std::cout<<integral<<std::endl;
	  //std::cout<<error<<std::endl;

     }
	 
     pthread_join( threadPWM, NULL);
     pthread_join( threadTAR, NULL);

     myfile.close();
     return 0; 
}

float read_func(){
    int fd;
    char buf[3];
    char val,value;
    float flight;
    fd=open("/dev/i2c-1",O_RDWR);
    if(fd<0)
    {
        printf("Error reading from file:%s\r\n",strerror(errno)); return 1;
    }
    if(ioctl( fd,I2C_SLAVE,I2C_ADDR)<0 )
    {
        printf("ioctl ERR: %s\r\n",strerror(errno));return 1;
    }
    val=0x01;
    if(write(fd,&val,1)<0)
    {
        printf("Error to power up\r\n");
    }
    val=0x11;
    if(write(fd,&val,1)<0)
    {
        printf("Starting the High-Resolution mode2\r\n");
    }
    usleep(200000);
    if(read(fd,&buf,3)){
        flight=(buf[0]*256+buf[1])*0.5/1.2;
        //printf("Luminum: %6.2flx\r\n",flight);
    }
    else{
        printf("Reading Error\r\n");
    }
    close(fd);
    return flight;
}
