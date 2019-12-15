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
using namespace std::chrono;

int u = 0;
float target = 0;

void errormsg(const char *msg)
{
    perror(msg);
    exit(1);
}

// Set software generated PWM
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

// Set user input
void* TAR(void* args)   
{
     while(1)
	  std::cin>>target;
}

int main(int argc, char *argv[])
{
     // Set LED output
     wiringPiSetup();
     pinMode(1, OUTPUT);
     pinMode(4, OUTPUT);
     
     // Set PID parameters
     float kp=0.1;
     float ki=0.2;
     float kd=0;
     float error;
     float actual=0;
     float integral = 0;
     float pre_error = 0;
     
     //Set socket receiving
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     
     //Read user fisrt target and open file for recording data
     target = std::stof(argv[2]);
     std::ofstream myfile;
     myfile.open ("data2.txt");
     
     // Start PWM thread.
     pthread_t  threadPWM;
     int ret1;
     
     ret1 = pthread_create(&threadPWM, NULL, PWM, NULL);
     
     if(ret1){
	  fprintf(stderr, "ERROR: Thread create returns :%d\n", ret1);
	  exit(EXIT_FAILURE);
     }
     
     // Start user input thread
     pthread_t  threadTAR;
     int ret2;
     
     ret2 = pthread_create(&threadTAR, NULL, TAR, NULL);
     
     if(ret2){
	  fprintf(stderr, "ERROR: Thread create returns :%d\n", ret2);
	  exit(EXIT_FAILURE);
     }
     
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     
     // Start socket service.
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
	  errormsg("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
	  sizeof(serv_addr)) < 0) 
	  errormsg("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
	  (struct sockaddr *) &cli_addr, 
	  &clilen);
     if (newsockfd < 0) 
          errormsg("ERROR on accept");
     bzero(buffer,256);
     
     /*   SYNC PART    */   
     printf("Syncing\n");
     milliseconds ts1 = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch());

     printf("Now time: %lld\n",ts1.count());

        
     strcpy(buffer, std::to_string(ts1.count()).c_str());
     
     long long int t1 = ts1.count();
     
     n = write(newsockfd,buffer,255);
     if (n < 0) errormsg("ERROR writing to socket");
     
     n = read(newsockfd,buffer,255);
     if (n < 0) errormsg("ERROR reading from socket");

     milliseconds ts4 = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch());
            
     long long int t4 = ts4.count();
            
     long long int t2 = atoll(buffer);

     long long int terr = t2 - ( t1 + (t4 - t1) / 2);
     
     printf("Time Error is %d\n", terr);
     
     long long int time_after_sync;
     
     long long int current_time;
     
     // Start main system loop
     while(1){
	  // Receive message
	  bzero(buffer,256);
	  n = read(newsockfd,buffer,255);
	  
	  milliseconds ms = duration_cast< milliseconds >(
	       system_clock::now().time_since_epoch());
               
	  current_time = ms.count() - t4;
	  
	  time_after_sync = ms.count() - terr;
	  if (n < 0) errormsg("ERROR reading from socket");
	  //printf("Here is the message: %s\ntime is %lld\ntime without sync is %lld\n",buffer, time_after_sync, ms.count());
         
	  actual = std::stof(buffer);
         
	  //printf("Actual value: %f\n", actual);
         
	  n = write(newsockfd,"ACK",3);
	  if (n < 0) errormsg("ERROR writing to socket");
          
	  // Compute PID output
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
	  //printf("Adjustment PWM u=%d\n", u);
	  
	  // Write to file or print to screen
	  //myfile<<current_time<<"	"<<actual<<"	"<<u<<std::endl;
	  std::cout<<current_time<<"	"<<actual<<"	"<<u<<std::endl;

     }
	 
     // Close all ports, threads and files
     pthread_join( threadPWM, NULL);
     pthread_join( threadTAR, NULL);
     close(newsockfd);
     close(sockfd);
     myfile.close();
     return 0; 
}

