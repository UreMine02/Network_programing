#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

char str[256];
char cmd[16], port[5], tmp[50], log_file[50];

void input() {
    printf("Nhap lenh: ");
    fgets(str, sizeof(str), stdin);

    int ret = sscanf(str, "%s%s%s%s", &cmd, &port, &log_file, &tmp);
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

    if (strcmp(cmd, "sv_server") != 0)
    {
        printf("ERROR sai ma lenh\n");
        return ;
    }
}

void repstr(char *str, char c)
{
    char *p = str;
    while(*p != NULL)
    {
        if (*p == '\n')
        {
            *p = c;
        }
        p++;
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
    printf("Waiting for conection from client: \n");
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
    FILE *fn = fopen(&log_file, "wb");
    while(1) {
        char arr[5][100];
        int size = sizeof(arr);
        int ret = recv(client, arr, size, 0);
        if (ret <= 0)
            break;
        printf("MSSV: %s", &arr[0]);
        printf("Ho va ten: %s", &arr[1]);
        printf("Ngay sinh: %s", &arr[2]);
        printf("CPA: %s", &arr[3]);
        time_t rawtime;
        struct tm * timeinfo;
        time(&rawtime);
        timeinfo = localtime (&rawtime);
        char timeNow[50];
        strftime(timeNow, sizeof(timeNow), "%Y-%m-%d %T", timeinfo);
        strcpy(arr[4], inet_ntoa(clientAddr.sin_addr));
        strcat(arr[4], " ");
        strcat(arr[4], timeNow);
        strcat(arr[4], " ");
        repstr(arr[0], ' ');
        strcat(arr[4], arr[0]);
        strcat(arr[4], " ");
        repstr(arr[1], ' ');
        strcat(arr[4], arr[1]);
        strcat(arr[4], " ");
        repstr(arr[2], ' ');
        strcat(arr[4], arr[2]);
        strcat(arr[4], " ");
        repstr(arr[3], ' ');
        strcat(arr[4], arr[3]);
        fprintf(fn, "%s \n", arr[4]);
    }
    fclose(fn);

    close(client);
    close(listener);
}