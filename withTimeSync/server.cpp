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
using namespace std::chrono;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");
     bzero(buffer,256);

     /*   SYNC PART    */   
     printf("Syncing\n");
     milliseconds ts1 = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch());

     printf("Now time: %lld\n",ts1.count());

        
     strcpy(buffer, std::to_string(ts1.count()).c_str());
     
     long long int t1 = ts1.count();
     
     n = write(newsockfd,buffer,255);
     if (n < 0) error("ERROR writing to socket");
     
     n = read(newsockfd,buffer,255);
     if (n < 0) error("ERROR reading from socket");

     milliseconds ts4 = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch());
            
     long long int t4 = ts4.count();
            
     long long int t2 = atoll(buffer);

     long long int terr = t2 - ( t1 + (t4 - t1) / 2);
     
     printf("Time Error is %d\n", terr);
     
     long long int time_after_sync;

     while(1){
         bzero(buffer,256);
         n = read(newsockfd,buffer,255);
         milliseconds ms = duration_cast< milliseconds >(
               system_clock::now().time_since_epoch());
               
         time_after_sync = ms.count() - terr;
         if (n < 0) error("ERROR reading from socket");
         printf("Here is the message: %s\ntime is %lld\ntime without sync is %lld\n",buffer, time_after_sync, ms.count());
         n = write(newsockfd,"ACK",3);
         if (n < 0) error("ERROR writing to socket");
     }

     close(newsockfd);
     close(sockfd);
     return 0; 
}
