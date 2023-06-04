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

#define NUMBER_PROCESS 9

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
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

    // Preforking
    for (int i = 0; i < NUMBER_PROCESS; ++i)
    {
        if (fork() == 0)
        {
            // Child process
            struct sockaddr_in clientAddr;
            int clientAddrLength = sizeof(clientAddr);
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
                printf("Receive from process %d: %s\n", getpid(), buff);

                char *message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Hello world!</h1></body></html>";
                send(clientSocket, message, strlen(message), 0);
                close(clientSocket);
            }

            exit(EXIT_SUCCESS);
        }
    }

    // Sleep main process
    getchar();

    // Close
    close(serverSocket);
    printf("Socket closed\n");

    printf("Main process closed\n");
    killpg(0, SIGKILL);
    return 0;
}