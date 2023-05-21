#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/select.h>

#define MAX_USER 64

bool queryDatabase(const char* file, const char* acc, const char* pass)
{
    FILE *f = fopen(file, "r");
    if (f == NULL)
    {
        printf("Open file failed\n");
        exit(1);
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

int main()
{
    // Check enough arguments
    // Argument contains ./run_file_name + port + db_file_name + file_print_cmd_output
    // if (argc != 4)
    // {
    //     printf("Missing arguments\n");
    //     exit(1);
    // }

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
    addr.sin_port = htons(9000);

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

    // Variable for logging client address
    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);

    // Declare and add socket to Fdset
    fd_set fdread, fdclone;
    FD_ZERO(&fdread);
    FD_SET(serverSocket, &fdread);

    // Listening
    char buff[256];
    int ret = 0;
    int numUsers = 0;
    int user_socket[MAX_USER];
    bool user_isLogged[MAX_USER];
    memset(user_isLogged, 0, sizeof(user_isLogged));

    while (1)
    {
        fdclone = fdread;
     
        ret = select(FD_SETSIZE, &fdclone, NULL, NULL, NULL);
        if (ret <= 0)
        {
            printf("Select Failed\n");
            break;
        }

        // Listen connection
        if (FD_ISSET(serverSocket, &fdclone))
        {
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);

            if (numUsers < MAX_USER)
            {
                printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                FD_SET(clientSocket, &fdread);
                user_socket[numUsers] = clientSocket;
                user_isLogged[numUsers] = false;
                numUsers++;

                char *message = "Connect successfully. Please login to continue (account password)\n";
                send(clientSocket, message, strlen(message), 0);
            }
            else
            {
                // Full connection
                printf("Exceed maximum connection\n");
                char *message = "Excees maximum connection\n";
                send(clientSocket, message, strlen(message), 0);
                close(clientSocket);
            }
        }

        // Listen message
        for (int i = 0; i < numUsers; ++i)
        {
            int client = user_socket[i];
            if (FD_ISSET(client, &fdclone))
            {
                ret = recv(client, buff, sizeof(buff), 0);
                buff[ret] = 0;
                if (buff[ret - 1] == '\n')
                    buff[ret - 1] = 0;
                printf("Receive from %d: %s\n", client, buff);

                if (ret <= 0)
                {
                    // Client is disconnected
                    user_socket[i] = user_socket[numUsers - 1];
                    user_isLogged[i] = user_isLogged[numUsers - 1];
                    numUsers--;

                    FD_CLR(client, &fdread);
                    close(client);

                    printf("Client %d has disconnected\n", client);
                    i--;
                    continue;
                }

                if (user_isLogged[i])
                {
                    // Operate command
                    char cmd[512], file_read[2048];
                    sprintf(cmd, "%s > %s", buff, "cmd_output.txt");
                    ret = system(cmd);
                    if (ret == -1)
                    {
                        char *message = "Command not found\n";
                        send(client, message, strlen(message), 0);
                        continue;
                    }

                    FILE *f = fopen("cmd_output.txt", "r");
                    if (f == NULL)
                    {
                        printf("Open File Failed\n");
                        exit(1);
                    }
                    while (!feof(f))
                    {
                        ret = fread(file_read, 1, sizeof(file_read), f);
                        if (ret <= 0)
                            break;
                        else
                            send(client, file_read, ret, 0);
                    }
                    send(client, "\n", 1, 0);
                    fclose(f);
                }
                else
                {
                    // Not logged in
                    char acc[32], pass[32], tmp[32];
                    ret = sscanf(buff, "%s%s%s", acc, pass, tmp);
                    if (ret == 2)
                    {
                        if (queryDatabase("user_database.txt", acc, pass))
                        {
                            char *message = "Login successfully\n";
                            send(client, message, strlen(message), 0);
                            user_isLogged[i] = true;
                        }
                        else
                        {
                            char *message = "Wrong account or password\n";
                            send(client, message, strlen(message), 0);
                        }
                    }
                    else
                    {
                        char *message = "Wrong format\n";
                        send(client, message, strlen(message), 0);
                    }
                }
            }
        }
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}