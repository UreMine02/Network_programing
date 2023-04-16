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
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }
    char filename[256];
    int name_size;
    recv(client, &name_size, sizeof(int), 0);

    recv(client, filename, name_size, 0);
    filename[name_size] = 0;

    char buf[2048]; 
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret < sizeof(buf))
            buf[ret] = 0;

        printf("Server: %s\n", buf);

    while (1)
    {
        printf("Enter string:");
        fgets(buf, sizeof(buf), stdin);

        if (strncmp(buf, "exit", 4) == 0)
            break;

        send(client, buf, strlen(buf), 0);

    }

    close(client);
}