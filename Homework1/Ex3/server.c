#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

char str[256];
char cmd[16], port[5], tmp[50], IPaddr[50];

void input() {
    printf("Nhap lenh: ");
    fgets(str, sizeof(str), stdin);

    int ret = sscanf(str, "%s%s%s%s", &cmd, &IPaddr, &port, &tmp);
    if (ret < 3)
    {
        printf("ERROR thieu tham so\n");
        return ;
    }

    if (ret > 3)
    {
        printf("ERROR thua tham so\n");
        return ;
    }

    if (strcmp(cmd, "tcp_server") != 0)
    {
        printf("ERROR sai ma lenh\n");
        return ;
    }
}
int main()
{
    input();
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(&port));
    printf("Waiting for connection from client: \n");
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    char arr[4][100];
    int size = sizeof(arr);
    int ret = recv(client, arr, size, 0);
    printf("MSSV: %s", &arr[0]);
    printf("Ho va ten: %s", &arr[1]);
    printf("Ngay sinh: %s", &arr[2]);
    printf("CPA: %s", &arr[3]);
    
    close(client);
    close(listener);
}