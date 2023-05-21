#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>

int main(int argc, char* argv[])
{
    // Check enough arguments
    // Argument contains ./run_file_name + id address + port
    // if (argc != 3)
    // {
    //     printf("Missing arguments\n");
    //     exit(1);
    // }

    // Create socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1)
    {
        perror("Create socket failed: ");
        exit(1);
    }

    // Connect to server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);
    
    if (connect(clientSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        printf("Connecting failed\n");
        exit(1);
    }
    printf("Connect Successfully\n");

    // Declare pollfd array
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;
    fds[1].fd = clientSocket;
    fds[1].events = POLLIN;

    // Listening
    char buff[256];
    while (1) 
    {
        int ret = poll(fds, 2, -1);
        if (ret < 0)
        {
            printf("Poll Failed\n");
            break;
        }

        // Scanf
        if (fds[0].revents & (POLLIN | POLLERR))
        {
            fgets(buff, sizeof(buff), stdin);
            if (buff[strlen(buff) - 1] == '\n')
                buff[strlen(buff) - 1] = 0;
            send(clientSocket, buff, strlen(buff), 0);
        }

        // Receive
        if (fds[1].revents & (POLLIN | POLLERR))
        {
            ret = recv(clientSocket, buff, sizeof(buff), 0);
            buff[ret] = 0;

            // Disconnected
            if (ret <= 0)
                break;

            printf("%s", buff);
        }
    }
    

    // Close socket
    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}