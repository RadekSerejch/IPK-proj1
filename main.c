#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include "string.h"
#include "unistd.h"


void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Storing start time
    clock_t start_time = clock();

    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

void CPU_usage(int in_soc) {
    //initialize values
    char buffer [200];
    unsigned long long int usertime, nicetime, systemtime, idletime;
    unsigned long long int ioWait, irq, softIrq, steal, guest, guestnice;
    unsigned long long int PrevIdle,Idle,PrevNonIdle,NonIdle;
    unsigned long long int prevusertime, prevnicetime, prevsystemtime, previdletime;
    unsigned long long int previoWait, previrq, prevsoftIrq, prevsteal, prevguest, prevguestnice;
    ioWait = irq = softIrq = steal = guest = guestnice = 0;
    previoWait = previrq = prevsoftIrq = prevsteal = prevguest = prevguestnice = 0;


    //first read from file
    FILE* file = fopen("/proc/stat", "r");
    if (file == NULL) {
        printf("Cannot open /proc/stat");
    }
    fgets(buffer, sizeof(buffer), file);
    (void) sscanf(buffer,   "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",&prevusertime, &prevnicetime, &prevsystemtime, &previdletime, &previoWait, &previrq, &prevsoftIrq, &prevsteal, &prevguest, &prevguestnice);
    fclose(file);

    //wait
    delay(200);

    //second read from file
    file = fopen("/proc/stat", "r");
    if (file == NULL) {
        printf("Cannot open /proc/stat");
    }
    fgets(buffer, sizeof(buffer), file);
    (void) sscanf(buffer,   "cpu  %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu %16llu",&usertime, &nicetime, &systemtime, &idletime, &ioWait, &irq, &softIrq, &steal, &guest, &guestnice);
    fclose(file);


    //calculate cpu usage
    PrevIdle = previdletime + previoWait;
    Idle = idletime + ioWait;
    PrevNonIdle = prevusertime + prevnicetime + prevsystemtime + previrq + prevsoftIrq + prevsteal;
    NonIdle = usertime + nicetime + systemtime + irq + softIrq + steal;
    unsigned long long int PrevTotal = PrevIdle + PrevNonIdle;
    unsigned long long int Total = Idle + NonIdle;
    unsigned long long int totald = Total - PrevTotal;
    unsigned long long int idled = Idle - PrevIdle;
    int usage = (int)((totald - idled)*100/totald);


    //send usage via socket
    char response [3];
    response[0] = usage%10 + '0';
    usage=usage/10;
    response[1] = usage%10 + '0';
    response[2] = '%';
    send(in_soc,response, 3,0);
}
void hostname(int in_soc){
    //read hostname from /proc/...
    char line [100];
    FILE *fp;
    fp= popen("cat /proc/sys/kernel/hostname", "r" );
    if (fp == NULL){
        printf("cannot proceed awk");
    }
     fgets( line, sizeof line, fp);
    int status = pclose(fp);
    if (status == -1){
        printf("error closing file fp");
    }

    //send hostname via socket
    send(in_soc,line, strlen(line),0);
}
void cpuname(int in_soc){
    //read hostname from /proc/...
    char line [100];
    FILE *fp;
    fp= popen("cat /proc/cpuinfo | grep \"model name\" | head -n 1 | awk -F':' '{ print $2 }'", "r" );
    if (fp == NULL){
        printf("cannot proceed awk");
    }
    fgets( line, sizeof line, fp);

    int status = pclose(fp);
    if (status == -1){
        printf("error closing file fp");
    }

    //send cpu name via socket
    send(in_soc,line, strlen(line),0);
}


int main(int argc, char **argv) {
    int in_socket;
    char* option_1 = "GET /hostname";
    char* option_2 = "GET /cpu-name";
    char* option_3 = "GET /load";
    char* response;


    //read port
    int port;
    if(argc != 2){
        printf("wrong parameters, need 1 parameter with number of port\n");
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[1]);


    //server
    //create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == 0){
        printf("error creating socket");
        exit(EXIT_FAILURE);
    }
    //set flags
    int flag = 1;
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) {
        printf("setsockopt fail");
        exit(EXIT_FAILURE);
    }
    if (-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(flag))) {
        printf("setsockopt fail");
        exit(EXIT_FAILURE);
    }
    //bind
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr *) &address,sizeof (address)) < 0){
        printf("bind error");
        exit(EXIT_FAILURE);
    }

    //listen
    if((listen(sockfd,100)) < 0 ){
        printf("listen error");
        exit(EXIT_FAILURE);
    }
    printf("server running on port: %d\n",port);
    //accept of messages
    unsigned int address_size = sizeof(address);
    while(1){
        in_socket= accept(sockfd, (struct sockaddr *) &address,&address_size);
        if(in_socket < 0){
            printf("%d\n",in_socket);
            printf("accept error");
            exit(EXIT_FAILURE);
        }
        else{
            char buff [1024];
            long res = 0;
            res = recv(in_socket,buff,1024,0);


            //switching actions
            if((strncmp(buff,option_1,strlen(option_1)) == 0) && buff[strlen(option_1)] == ' ' ){
                //hostname
                response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n";
                send(in_socket,response, strlen(response),0);
                hostname(in_socket);
            }else if((strncmp(buff,option_2,strlen(option_2)) == 0) && buff[strlen(option_2)] == ' '){
                //cpu-name
                response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\ncpu-name";
                send(in_socket,response, strlen(response),0);
                cpuname(in_socket);
            }else if((strncmp(buff,option_3,strlen(option_3)) == 0) && buff[strlen(option_3)] == ' '){
                //load
                response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain;\r\n\r\n";
                send(in_socket,response, strlen(response),0);
                CPU_usage(in_socket);
            }else{
                //else
                response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain;\r\n\r\nError 404\r\nbad request";
                send(in_socket,response, strlen(response),0);
            }
            close(in_socket);

            //read rest of message
            for(;;){
                res = recv(in_socket,buff,1024,0);
                if(res <= 0)
                    break;
            }

        }
    }
}
