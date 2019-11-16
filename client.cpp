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
#define I2C_ADDR 0x23

using namespace std::chrono;

int interval = 100;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
float read_func();
int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
        
        
    bzero(buffer,256);
    
    /* SYNCING */
    std::ostringstream oss;
    n = read(sockfd,buffer,255);
    if (n < 0) 
        error("ERROR reading from socket");
    
    milliseconds ms = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch());

    oss << ms.count();
    bzero(buffer,256);
    
    strcpy(buffer, oss.str().c_str());
    
    n = write(sockfd,buffer,strlen(buffer));
            
    if (n < 0) 
        error("ERROR writing to socket");

    while(1){
        bzero(buffer,256);
        auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);

        time_t my_time = time(NULL); 

        char lum[10];
        snprintf(lum, 10, "%f", read_func());
        
        strcpy(buffer, lum);
        
        strcat(buffer, " ");

        std::this_thread::sleep_until(x);
        
        milliseconds ms = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch());

        printf("%lld",ms.count());

        oss << ms.count();
        
        strcat(buffer, oss.str().c_str());
        
        while(strcmp(buffer,"ACK")!=0){
            
            n = write(sockfd,buffer,strlen(buffer));
            
            if (n < 0) 
                error("ERROR writing to socket");
            bzero(buffer,256);
            n = read(sockfd,buffer,255);
            if (n < 0) 
                error("ERROR reading from socket");
        }
        
        oss.str("");
        oss.clear();
        
     }

        
    close(sockfd);
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
    return flight;
}
