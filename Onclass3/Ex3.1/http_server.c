#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

void *thread_proc(void *);

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

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

    int num_threads = 8;
    for (int i = 0; i < num_threads; i++)
    {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, thread_proc, &listener);
        pthread_detach(thread_id);
    }

    while (1){}
    
    close(listener);    

    return 0;
}

void *thread_proc(void *param)
{
    int listener = *(int *)param;
    char buf[2048];

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            continue;
        if (ret < sizeof(buf))
            buf[ret] = 0;
        printf("%s\n", buf);
        strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao</h1></body></html>");
        send(client, buf, strlen(buf), 0);
        close(client);
    }
}