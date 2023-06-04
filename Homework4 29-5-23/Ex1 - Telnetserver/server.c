#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdbool.h>

void signalHandler(int sign)
{
    int status;
    pid_t pid = wait(&status);

    if (status == EXIT_SUCCESS)
        printf("Child process %d terminated\n", pid);
}

bool login(char *file, char *acc, char *pass)
{
    FILE *f = fopen(file, "r");
    if (f == NULL)
    {
        printf("Open file database failed\n");
        return false;
    }

    char buff[256];
    char* ret;
    char _acc[32], _pass[32];
    while (!feof(f))
    {
        ret = fgets(buff, sizeof(buff), f);
        if (ret == NULL)
            break;

        sscanf(buff, "%s%s", _acc, _pass);
        if (strcmp(acc, _acc) == 0)
        {
            if (strcmp(pass, _pass) == 0)
            {
                fclose(f);
                return true;
            }
        }
    }

    fclose(f);
    return false;
}

int main(int argc, char* argv[])
{
    // Check enough arguments
    // Argument contains ./run_file_name + port + db_user_file + cmd_log_file
    if (argc != 4)
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

    // For logging address of connection
    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);
    
    // Listening
    signal(SIGCHLD, signalHandler);
    while (1)
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        if (fork() == 0)
        {
            // Child process
            close(serverSocket);
            char buff[256];
            bool isLogged = false;

            // Send login request
            sprintf(buff, "Please login to continue\n");
            send(clientSocket, buff, strlen(buff), 0);

            // Listening
            while (1)
            {
                int ret = recv(clientSocket, buff, sizeof(buff), 0);
                if (ret <= 0)
                    break;
                buff[ret] = 0;
                if (buff[ret - 1] == '\n')
                    buff[ret - 1] = 0;

                if (!isLogged)
                {
                    char acc[32], pass[32], tmp[32];
                    ret = sscanf(buff, "%s%s%s", acc, pass, tmp);
                    if (ret == 2)
                    {
                        if (login(argv[2], acc, pass))
                        {
                            isLogged = true;
                            sprintf(buff, "Login successfully\n");
                            send(clientSocket, buff, strlen(buff), 0);
                        }
                        else
                        {
                            sprintf(buff, "Wrong account or password\n");
                            send(clientSocket, buff, strlen(buff), 0);
                        }
                    }
                    else
                    {
                        sprintf(buff, "Wrong format\n");
                        send(clientSocket, buff, strlen(buff), 0);
                    }
                }
                else 
                {
                    printf("Receive from pid %d: %s\n", getpid(), buff);

                    // Operate command
                    char cmd[512], file_read[2048];
                    sprintf(cmd, "%s > %s", buff, argv[3]);
                    ret = system(cmd);
                    if (ret == -1)
                    {
                        char *message = "Command not found\n";
                        send(clientSocket, message, strlen(message), 0);
                        continue;
                    }

                    FILE *f = fopen(argv[3], "r");
                    if (f == NULL)
                    {
                        printf("Open File Failed\n");
                        break;
                    }

                    while (!feof(f))
                    {
                        ret = fread(file_read, 1, sizeof(file_read), f);
                        if (ret <= 0)
                            break;
                        else
                            send(clientSocket, file_read, ret, 0);
                    }
                    send(clientSocket, "\n", 1, 0);
                    fclose(f);    
                }
            }

            // Terminated process
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
    printf("Process closed\n");
    killpg(0, SIGKILL);
    return 0;
}