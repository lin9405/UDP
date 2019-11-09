#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>


#define BUFSIZE   500

#ifndef boolean
# define boolean
typedef enum { false, true } bool;
#endif /* ifndef boolean */

int main(int argc, char *argv[]) {

    if (2 != argc) {
        printf("Usage: %s <port>\n", *argv);
        exit(1);
    }

    int serverSock;
    struct sockaddr_in serverAdr;

    assert(-1 != (serverSock = socket(PF_INET, SOCK_DGRAM, 0)));

    bzero(&serverAdr, sizeof(serverAdr));
    serverAdr.sin_family      = AF_INET;
    serverAdr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAdr.sin_port        = htons(atoi(argv[1]));

    assert(-1 != bind(serverSock, (const struct sockaddr *)&serverAdr, sizeof(serverAdr)));
    //printf("server start\n");

    while (1) {
        struct sockaddr_in clientAdr;
        socklen_t clientAdrSize;

        char filename[BUFSIZE - 1];
        char rcvBuff[BUFSIZE];
        int  dataSize  = 0;
        int  dataSeq   = 0;
        int  rcvedSize = 0;
        int  i         = 0;
        const char eof = 0x1A;
        bool isFirst   = true;
        bool isValid   = false;

        while (1) {
            clientAdrSize = sizeof(clientAdr);
            assert(-1 != (dataSize = recvfrom(serverSock, rcvBuff, BUFSIZE, 0, (struct sockaddr *)&clientAdr, &clientAdrSize)));

            if (*(rcvBuff) == eof) {
                break;
            }

            if ((rcvBuff[0] == '0') && !isValid) {
                for (i = 0; i < BUFSIZE - 1; i++) {
                    rcvBuff[i] = rcvBuff[i + 1];
                }
                strcpy(filename, rcvBuff);
                isValid = true;
            } else if (((rcvBuff[0] == '1') || (rcvBuff[0] == '2')) && isValid) {
                rcvedSize += dataSize;

                FILE *fp = NULL;
                long int offset;

                if (isFirst == true) {
                    assert(fp = fopen(filename, "w"));
                    isFirst = false;
                } else assert(fp = fopen(filename, "a"));

                offset = BUFSIZE * dataSeq;

                fseek(fp, offset, SEEK_SET);

                for (i = 0; i < BUFSIZE - 1; i++) {
                    rcvBuff[i] = rcvBuff[i + 1];
                }
                fwrite(rcvBuff, dataSize, 1, fp);

                fclose(fp);
                dataSeq++;
            }
        }
    }
    close(serverSock);
    return 0;
}
