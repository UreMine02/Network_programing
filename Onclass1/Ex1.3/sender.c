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
    char str[512], filesend[256], IPaddr[64], tmp[64];
    int port;

    printf("Nhap ten file, dia chi va cong: ");
    fgets(str, sizeof(str), stdin);
    int pt = sscanf(str, "%s%s%d%s", filesend, IPaddr, &port, tmp);

    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);


    char buf[256];
    FILE *f = fopen(filesend,"rb");

    //
    strcpy(buf, filesend);
    sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr));

    int ret;

    while (!feof(f))
    {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0) {
            break;
        }
        sendto(sender, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr));
    }
    
    fclose(f);
    char buf2[256] = "close";
    sendto(sender, buf2, strlen(buf2), 0, (struct sockaddr *)&addr, sizeof(addr));
    close(sender);
}