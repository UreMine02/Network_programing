#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Server nhan file tu client
char str[256];
char cmd[16], port[5], send_name[100], new_send[100], tmp[10];

void input() {
    printf("Nhap lenh: ");
    fgets(str, sizeof(str), stdin);

    int ret = sscanf(str, "%s%s%s%s%s", &cmd, &port, &send_name, &new_send, &tmp);
    if (ret < 4)
    {
        printf("ERROR thieu tham so\n");
        return ;
    }

    if (ret > 4)
    {
        printf("ERROR thua tham so\n");
        return ;
    }

    if (strcmp(cmd, "tcp_server") != 0)
    {
        printf("ERROR sai lenh\n");
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

    printf("Liten to client: \n");
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));

    char *filenamesend = &send_name;

    FILE *f = fopen(filenamesend, "rb");

    char buf[2048];

    int name_size = strlen(filenamesend);
    send(client, &name_size, sizeof(int), 0);
    strcpy(buf, filenamesend);
    send(client, buf, strlen(filenamesend), 0);

    while (!feof(f))
    {
        ret = fread(buf, 1, sizeof(buf), f);
        if (ret <= 0)
            break;
        send(client, buf, ret, 0);
    }

    fclose(f);

    char *filenewsend = &new_send;

    FILE *fn = fopen(filenewsend, "wb");
    char buf2[2048];

    while (1)
    {
        int ret2 = recv(client, buf2, sizeof(buf2), 0);
        if (ret2 <= 0)
            break;
        fwrite(buf2, 1, ret2, fn);
    }

    fclose(fn);
    
    close(client);
    close(listener);
}