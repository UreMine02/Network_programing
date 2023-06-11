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
#include <stdbool.h>

char file_db[64], file_cmd[64];
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

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

void *threadPerClient(void *arg)
{
    bool isLogged = false;
    int client = *(int*)arg;
    char buff[256], sendBuff[512];
    while (1)
    {
        int ret = recv(client, buff, sizeof(buff), 0);
        if (ret <= 0)
            break;

        buff[ret] = 0;
        if (buff[ret - 1] == '\n')
            buff[ret - 1] = 0;

        if (!isLogged)
        {
            char acc[32], pass[32], tmp[32];
            ret = sscanf(buff, "%s%s%s", acc, pass, tmp);
            if (ret != 2)
            {
                // Wrong format
                char *message = "Wrong format (acc pass)\n";
                send(client, message, strlen(message), 0);
                continue;
            }

            if (!login(file_db, acc, pass))
            {
                // Wrong format
                char *message = "Wrong account or password\n";
                send(client, message, strlen(message), 0);
                continue;
            }

            // Right format "client_id: client_name"
            char *message = "Login successfully\n";
            send(client, message, strlen(message), 0);
            isLogged = true;
        }
        else
        {
            // Logged in
            char cmd[512], file_read[2048];
            sprintf(cmd, "%s > %s", buff, file_cmd);
            
            // Mutex
            pthread_mutex_lock(&file_mutex);

            system(cmd);
            FILE *f = fopen(file_cmd, "r");
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
                    send(client, file_read, ret, 0);
            }
            send(client, "\n", 1, 0);
            fclose(f);   

            pthread_mutex_unlock(&file_mutex);
            // End mutex
        }
    }

    // Close client
    printf("Client %d disconnected\n", client);
    close(client);
}

int main(int argc, char* argv[])
{
    // Check enough arguments
    // Argument contains ./run_file_name + port + file_user_db + file_cmd_log
    if (argc != 4)
    {
        printf("Missing arguments\n");
        exit(1);
    }
    strcpy(file_db, argv[2]);
    strcpy(file_cmd, argv[3]);

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

    // Declare sockaddr to log client's address
    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);
    pthread_t thread_id;

    // Listening
    while (1) 
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        pthread_create(&thread_id, NULL, threadPerClient, (void *)&clientSocket);
        pthread_detach(thread_id);
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}