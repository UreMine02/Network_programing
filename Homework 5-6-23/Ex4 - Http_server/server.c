#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define NUMBER_PRETHREADING 2

int serverSocket;

void *preThreading(void *arg)
{
    // For logging address of connection
    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);

    // Listening
    char buff[256];
    while (1)
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        int ret = recv(clientSocket, buff, sizeof(buff), 0);
        if (ret <= 0)
        {
            close(clientSocket);
            continue;
        }
        buff[ret] = 0;
        printf("Receive from %s:%d\n%s\n",inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), buff);

        char *message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello world!</h1></body></html>";
        send(clientSocket, message, strlen(message), 0);
        close(clientSocket);
    }
}

int main(int argc, char* argv[])
{
    // Check enough arguments
    // Argument contains ./run_file_name + port
    if (argc != 2)
    {
        printf("Missing arguments\n");
        exit(1);
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == -1)
    {
        perror("Create socket failed: ");
        exit(1);
    }

    // Create struct sockaddr
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);;
    addr.sin_port = htons(atoi(argv[1]));

    // Bind socket to sockaddr
    if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Binding Failed\n");
        exit(1);
    }

    // Listen 
    if (listen(serverSocket, 5) == -1)
    {
        printf("Listening Failed\n");
        exit(1);
    }
    printf("Waiting for client connecting ...\n");

    // Prethreading
    pthread_t thread_id[NUMBER_PRETHREADING];
    for (int i = 0; i < NUMBER_PRETHREADING; ++i)
    {
        pthread_create(&thread_id[i], NULL, preThreading, NULL);
    }

    for (int i = 0; i < NUMBER_PRETHREADING; ++i)
    {
        pthread_join(thread_id[i], NULL);
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}