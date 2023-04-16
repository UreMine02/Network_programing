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
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;


    char str[256];
    char IP_addr[16], cmd[16], tmp[16];
    int port;

    printf("Nhap lenh: ");
    fgets(str, sizeof(str), stdin);

    int pt = sscanf(str, "%s%s%d%s", cmd, IP_addr, &port, tmp);
    if (pt < 3)
    {
        printf("ERROR thieu tham so\n");
        return 1;
    }

    if (pt > 3)
    {
        printf("ERROR thua tham so\n");
        return 1;
    }

    addr.sin_addr.s_addr = inet_addr(IP_addr);
    addr.sin_port = htons(port);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }
    char buf[256];
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        printf("Connection closed\n");
        return 1;
    }

    while (1)
    {
        printf("Enter data:");
        fgets(buf, sizeof(buf), stdin);

        send(client, buf, strlen(buf), 0);

        if (strncmp(buf, "exit", 4) == 0)
            break;
    }

    close(client);
}