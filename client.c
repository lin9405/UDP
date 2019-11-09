#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>

#define BUFSIZE 500 

int main(int argc, char **argv)
{
    if (4 != argc){
        printf("Usage: %s <IP> <PORT> <FILENAME>\n", *argv);
        exit(1);
    }
    
    int clientSock;
    struct sockaddr_in serverAdr;
    
    FILE* fp = NULL;
    size_t fSize;
    
    int sendSize = 0;
    int seqnum = 0;
    const char eof = 0x1A;
    
    assert(-1 != (clientSock = socket(PF_INET, SOCK_DGRAM, 0)));
    
    bzero(&serverAdr, sizeof(serverAdr));
    serverAdr.sin_family = AF_INET;
    serverAdr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAdr.sin_port = htons(atoi(argv[2]));
    
    
    assert(fp = fopen(argv[3], "r"));
    
    
    fseek(fp, 0, SEEK_END);
    fSize = ftell(fp); 
    fseek(fp, 0, SEEK_SET);
    
    printf("Start\n");
    char sndBuff[BUFSIZE] = "";
    char temp[BUFSIZE - 1] = "";
    strcpy(sndBuff, "0");
    strcat(sndBuff, argv[3]);
    int i = 0;
    for(i = 0 ; i< 50; i++){
        sendto(clientSock, sndBuff, BUFSIZE, 0,(const struct sockaddr*)&serverAdr, sizeof(serverAdr));
    }
    
    while(fSize > 0) {
        sendSize = fSize > BUFSIZE ? BUFSIZE : fSize;
        
        if(fSize > BUFSIZE){
            strcpy(sndBuff, "1");
        } else{
            strcpy(sndBuff, "2");
        }
        

        fread(temp, sendSize - 1, 1, fp);
        strcat(sndBuff, temp);

        sendto(clientSock, sndBuff, sendSize, 0, (const struct sockaddr*)&serverAdr, sizeof(serverAdr));
        
        fSize -= sendSize;
        
        seqnum++;
    }

    for(i=0;i<50;i++){
        sendto(clientSock, &eof, sizeof(eof), 0, (const struct sockaddr*)&serverAdr, sizeof(serverAdr));
    }
    printf("Success\n");
    fclose(fp);
    
    close(clientSock);
    
    return 0;
}

