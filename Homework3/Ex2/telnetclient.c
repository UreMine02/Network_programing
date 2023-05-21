#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main()
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

    // Declare fdset
    fd_set fdread, fdclone;
    FD_ZERO(&fdread);
    FD_SET(STDIN_FILENO, &fdread);
    FD_SET(clientSocket, &fdread);

    // Listening
    char buff[256];
    int ret;
    while (1)
    {
        fdclone = fdread;
        ret = select(clientSocket + 1, &fdclone, NULL, NULL, NULL);

        if (ret <= 0)
        {
            printf("Select Failed\n");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fdclone))
        {
            fgets(buff, sizeof(buff), stdin);
            buff[strlen(buff) - 1] = 0;
            send(clientSocket, buff, strlen(buff), 0);
        }

        if (FD_ISSET(clientSocket, &fdclone))
        {
            ret = recv(clientSocket, buff, sizeof(buff), 0);
            buff[ret] = 0;
            if (ret <= 0)
                // Disconnected
                break;

            printf("%s", buff);
        }
    } 
    

    // Close socket
    close(clientSocket);
    printf("Socket closed\n");
    return 0;
}