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
#include <dirent.h>
#include <stdbool.h>

void signalHandler(int signo) 
{
    int stat;
    pid_t pid = wait(&stat);
    if (stat == EXIT_SUCCESS)
    {
        printf("Child %d terminated\n", pid);
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

    // Accept connection
    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);
    
    // Listening
    signal(SIGCHLD, signalHandler);
    char buff[512];
    while (1)
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
    
        if (fork() == 0)
        {
            close(serverSocket);
            char *msg = "Connecting successfully\n";
            send(clientSocket, msg, strlen(msg), 0);

            DIR *dir = opendir(".");
            struct dirent *entry;
            int count = 0;
            char buff[1024];
            char *file_list[512];

            while ((entry = readdir(dir)) != NULL)
            {
                if (entry->d_type == DT_REG)
                {
                    file_list[count] = (char *) malloc(strlen(entry->d_name) + 1);
                    strcpy(file_list[count], entry->d_name);
                    count++;
                }
            }
            count--;
            sprintf(buff, "OK %d\r\n", count);
            for (int i = 0; i < count; ++i)
            {
                char tmp[64];
                sprintf(tmp, "%s\r\n", file_list[i]);
                strcat(buff, tmp);
            }
            strcat(buff, "\r\n");

            if (count == 0)
            {
                char *msg = "ERROR No files to download \r\n";
                send(clientSocket, msg, strlen(msg), 0);
                close(clientSocket);
                exit(EXIT_SUCCESS);
            }
            else
            {
                send(clientSocket, buff, strlen(buff), 0);
                while (1)
                {
                    int ret = recv(clientSocket, buff, sizeof(buff), 0);
                    if (ret <= 0)
                        break;
                    buff[ret] = 0;
                    if (buff[ret - 1] == '\n')
                        buff[ret - 1] = 0;

                    FILE *f = fopen(buff, "rb");
                    if (f == NULL)
                    {
                        char *msg = "ERROR No files to download\r\n";
                        send(clientSocket, msg, strlen(msg), 0);
                        fclose(f);
                        continue;
                    }

                    fseek(f, 0, SEEK_END);
                    long file_size = ftell(f);
                    fseek(f, 0, SEEK_SET);

                    char msg[32], buff[2048];
                    sprintf(msg, "OK %ld\r\n", file_size);
                    send(clientSocket, msg, strlen(msg), 0);

                    while (!feof(f))
                    {
                        ret = fread(buff, 1, sizeof(buff), f);
                        if (ret <= 0) break;
                        send(clientSocket, buff, ret, 0);
                    }

                    send(clientSocket, "\r\n", 2, 0);
                    fclose(f);
                    break;
                }
            }

            close(clientSocket);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(clientSocket);
        }
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    killpg(0, SIGKILL);
    return 0;
}