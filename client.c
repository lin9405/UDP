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

#define BUFSIZE 500 // MAX MTU

int main(int argc, char **argv)
{
    if (4 != argc){
        printf("Usage: %s <IP> <PORT> <FILENAME>\n", *argv);
        exit(1);
    }
    
    int clnt_sock;
    struct sockaddr_in serv_Addr;
    
    FILE* fp = NULL;
    size_t fSize;
    
    int sendSize = 0;
    int seqnum = 0;
    const char eof = 0x1A;
    
    assert(-1 != (clnt_sock = socket(PF_INET, SOCK_DGRAM, 0)));
    
    bzero(&serv_Addr, sizeof(serv_Addr));
    serv_Addr.sin_family = AF_INET;
    serv_Addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_Addr.sin_port = htons(atoi(argv[2]));
    
    
    //보낼 file open
    assert(fp = fopen(argv[3], "r"));
    
    
    fseek(fp, 0, SEEK_END);
    fSize = ftell(fp); //보낼 전체 데이터 양.
    fseek(fp, 0, SEEK_SET);
    
    printf("Start...\n");
    char sndBuff[BUFSIZE] = "";
    char temp[BUFSIZE - 1] = "";
    strcpy(sndBuff, "0");
    strcat(sndBuff, argv[3]);
    int i = 0;
    for(i = 0 ; i< 50; i++){
        sendto(clnt_sock, sndBuff, BUFSIZE, 0,(const struct sockaddr*)&serv_Addr, sizeof(serv_Addr));
    }
    
    while(fSize > 0) {
        sendSize = fSize > BUFSIZE ? BUFSIZE : fSize;
        
        //BUFSIZE보다 fSize가 작아지면 마지막 pkt
        // 0:name 1:data 2:last seq
        if(fSize > BUFSIZE){
            strcpy(sndBuff, "1");
        } else{
            strcpy(sndBuff, "2");
        }
        
        //sndPkt.dataSize만큼의 데이터를 읽어옴
        fread(temp, sendSize - 1, 1, fp);
        strcat(sndBuff, temp);
        
        //pkt전송.
        sendto(clnt_sock, sndBuff, sendSize, 0, (const struct sockaddr*)&serv_Addr, sizeof(serv_Addr));
        
        fSize -= sendSize;
        
        seqnum++;
    }
    //마지막 eof전송.
    for(i=0;i<50;i++){
        sendto(clnt_sock, &eof, sizeof(eof), 0, (const struct sockaddr*)&serv_Addr, sizeof(serv_Addr));
    }
    printf("Done...\n");
    fclose(fp);
    
    close(clnt_sock);
    
    return 0;
}

