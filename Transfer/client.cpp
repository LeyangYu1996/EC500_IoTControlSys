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

using namespace std::chrono;

int interval = 1000;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

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
    //printf("Please enter the message: ");
    bzero(buffer,256);
    //fgets(buffer,255,stdin);
    std::ostringstream oss;
    while(1){
        auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);

        time_t my_time = time(NULL); 

        milliseconds ms = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch());

        printf("%lld",ms.count());

        oss << ms.count();
        //strcpy(buffer, ctime(&my_time));
        strcpy(buffer, oss.str().c_str());
        
        oss.str("");
        oss.clear();

        while(strcmp(buffer,"ACK")!=0){
            n = write(sockfd,buffer,strlen(buffer));
            //sendSig();
            if (n < 0) 
                error("ERROR writing to socket");
            bzero(buffer,256);
            n = read(sockfd,buffer,255);
        }
        
        if (n < 0) 
             error("ERROR reading from socket");

        std::this_thread::sleep_until(x);
        //printf("%s\n",buffer);
     }

    close(sockfd);
    return 0;
}
