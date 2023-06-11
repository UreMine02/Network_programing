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
#include <time.h>

#define MAX_USER 64

int clients[MAX_USER];
int numberOfClients = 0;
pthread_mutex_t numberOfClients_mutex = PTHREAD_MUTEX_INITIALIZER;
time_t t;
struct tm tm;

void *threadPerClient(void *arg)
{
    bool isLogged = false;
    int client = *(int*)arg;
    char client_name[32], buff[256], sendBuff[512];
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
            char cmd[32], client_id[32], tmp[32];
            ret = sscanf(buff, "%s%s%s", cmd, client_id, tmp);
            if (ret != 2)
            {
                // Wrong format
                char *message = "Wrong format (client_id: client_name)\n";
                send(client, message, strlen(message), 0);
                continue;
            }

            if (strcmp(cmd, "client_id:") != 0)
            {
                // Wrong format
                char *message = "Wrong format (client_id: client_name)\n";
                send(client, message, strlen(message), 0);
                continue;
            }

            // Right format "client_id: client_name"
            strcpy(client_name, client_id);
            char *message = "Login successfully\n";
            send(client, message, strlen(message), 0);
            isLogged = true;
        }
        else
        {
            // Logged in
            sprintf(sendBuff, "%04d/%02d/%02d %02d:%02d:%02d %s:%s\n", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, 
            tm.tm_hour, tm.tm_min, tm.tm_sec, client_name, buff);
            
            // Mutex
            pthread_mutex_lock(&numberOfClients_mutex);
            
            for (int i = 0; i < numberOfClients; i++)
            {
                if (clients[i] != client)
                {
                    send(clients[i], sendBuff, strlen(sendBuff), 0);
                }
            }

            pthread_mutex_unlock(&numberOfClients_mutex);
            // End mutex
        }
    }

    // Close client
    // Mutex
    pthread_mutex_lock(&numberOfClients_mutex);
    
    for (int i = 0; i < numberOfClients; i++)
    {
        if (clients[i] == client)
        {
            clients[i] = clients[numberOfClients - 1];
            numberOfClients--;
            break;
        }
    }

    pthread_mutex_unlock(&numberOfClients_mutex);
    // End mutex

    printf("Client %d disconnected\n", client);
    close(client);
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

    // Declare sockaddr to log client's address
    struct sockaddr_in clientAddr;
    int clientAddrLength = sizeof(clientAddr);
    pthread_t thread_id;
    t = time(NULL);
    tm = *localtime(&t);

    // Listening
    while (1) 
    {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        
        if (numberOfClients < MAX_USER)
        {
            printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            
            // Mutex
            pthread_mutex_lock(&numberOfClients_mutex);
            clients[numberOfClients] = clientSocket;
            numberOfClients++;
            pthread_mutex_unlock(&numberOfClients_mutex);
            // End mutex

            pthread_create(&thread_id, NULL, threadPerClient, (void *)&clientSocket);
            pthread_detach(thread_id);
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

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}