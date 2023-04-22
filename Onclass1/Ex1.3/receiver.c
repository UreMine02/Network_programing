#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int port;


    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    bind(receiver, (struct sockaddr *)&addr, sizeof(addr));

    struct sockaddr_in sender_addr;
    int sender_addr_len = sizeof(sender_addr);


    FILE *f = fopen("writefile.txt","wb");
    FILE * fp = NULL;
    
    
    while(1)
    {
        char buf[64];
        int ret = recvfrom(receiver, buf, sizeof(buf), 0,(struct sockaddr *)&sender_addr, &sender_addr_len);
        if (ret < sizeof(buf))
            buf[ret] = 0;
        if(strcmp(buf, "close") == 0) {
            break;
        }
        fwrite(buf, 1, ret, f);
        fwrite("\n", 1, 1, f); 
    }
    fclose(f);
    close(receiver);
}