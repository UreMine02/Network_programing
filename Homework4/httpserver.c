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
#include <limits.h>
#include <dirent.h>

int port;

char* getParentPath(char *path)
{
    char *ptr = strrchr(path, '/');
    if (ptr == path)
    {
        return NULL;
    }
    else
    {
        int length = ptr - path;
        char *parent = malloc(length + 1);
        memcpy(parent, path, length);
        parent[length] = 0;
        return parent;
    }
}

void signal_handler(int signo)
{
    wait(NULL);
}

void *client_thread(void *param)
{
    int clientSocket = *(int *)param;
    char buff[2048];

    int ret = recv(clientSocket, buff, sizeof(buff) - 1, 0);
    if (ret <= 0)
    {
        close(clientSocket);
        return NULL;
    }
    buff[ret] = 0;
    printf("\n%s\n\n", buff);

    if (strncmp(buff, "GET /get", 8) == 0)
    {
        // Request with query
        char *pos1 = strchr(buff, '?') + 1;
        char *pos2 = strchr(pos1, ' ');
        int queryLength = pos2 - pos1;
        char *query = malloc(queryLength + 1);
        memcpy(query, pos1, queryLength);
        query[queryLength] = 0;

        if (strncmp(query, "file=", 5) == 0)
        {
            // Is file
            char *file = query + 5;
            char *extension = strrchr(file, '.');
            if (extension == NULL)
            {
                char *message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html><h1>File type not support</h1></html>";
                send(clientSocket, message, strlen(message), 0);
                close(clientSocket);
                return NULL;
            }
            if ((strcmp(extension, ".txt") == 0) || (strcmp(extension, ".c") == 0) || (strcmp(extension, ".cpp") == 0))
            {
                char *header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n";
                send(clientSocket, header, strlen(header), 0);
                FILE *f = fopen(file, "rb");
                if (f == NULL)
                {
                    return NULL;
                }
                while (!feof(f))
                {
                    int ret = fread(buff, 1, sizeof(buff), f);
                    if (ret <= 0) break;
                    send(clientSocket, buff, ret, 0);
                }
                fclose(f);
            }
            else if ((strcmp(extension, ".jpg") == 0) || (strcmp(extension, ".png") == 0))
            {
                char *header = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nConnection: close\r\n\r\n";
                send(clientSocket, header, strlen(header), 0);
                FILE *f = fopen(file, "rb");
                if (f == NULL)
                {
                    return NULL;
                }
                while (!feof(f))
                {
                    int ret = fread(buff, 1, sizeof(buff), f);
                    if (ret <= 0) break;
                    send(clientSocket, buff, ret, 0);
                }
                fclose(f);
            }
            else if ((strcmp(extension, ".mp3") == 0) || (strcmp(extension, ".mp4") == 0))
            {
                FILE *f = fopen(file, "rb");
                if (f == NULL)
                {
                    return NULL;
                }
                fseek(f, 0, SEEK_END);
                long fsize = ftell(f);
                fseek(f, 0, SEEK_SET);

                char response_header[2048];
                sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: audio/mp3\r\nConnection: close\r\n\r\n", fsize);
                send(clientSocket, response_header, strlen(response_header), 0);

                while (1)
                {
                    int len = fread(buff, 1, sizeof(buff), f);
                    if (len <= 0)
                        break;
                    send(clientSocket, buff, len, 0);
                }
                fclose(f);
            }
            else if ((strcmp(extension, ".pdf") == 0))
            {
                char *header = "HTTP/1.1 200 OK\r\nContent-Type: application/pdf\r\nConnection: close\r\n\r\n";
                send(clientSocket, header, strlen(header), 0);
                FILE *f = fopen(file, "rb");
                if (f == NULL)
                {
                    return NULL;
                }
                while (!feof(f))
                {
                    int ret = fread(buff, 1, sizeof(buff), f);
                    if (ret <= 0) break;
                    send(clientSocket, buff, ret, 0);
                }
                fclose(f);
            }
            else
            {
                char *message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html><h1>File type not support</h1></html>";
                send(clientSocket, message, strlen(message), 0);
                close(clientSocket);
                return NULL;
            }
        }
        else if (strncmp(query, "dir=", 4) == 0)
        {
            // Is dir
            char *directory = query + 4;
            DIR *dir = opendir(directory);
            struct dirent *entry;

            char *message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html>";
            send(clientSocket, message, strlen(message), 0);
            sprintf(buff, "<h1><a style=\"font-weight:900;color:#00008B\" href=\"http://localhost:%d/get?dir=%s\">.</a></h1>", port, directory);
            send(clientSocket, buff, strlen(buff), 0);
            char *tmp;
            if ((tmp = getParentPath(directory)) != NULL)
            {
                sprintf(buff, "<h1><a style=\"font-weight:900;color:#00008B\" href=\"http://localhost:%d/get?dir=%s\">..</a></h1>", port, tmp);
                send(clientSocket, buff, strlen(buff), 0);
            }

            while ((entry = readdir(dir)) != NULL)
            {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
                {
                    if (entry->d_type == DT_REG)
                    {
                        sprintf(buff, "<h1><i><a style=\"font-weight:100;color:#00d2ff\" href=\"http://localhost:%d/get?file=%s/%s\">%s</a></i></h1>", port, directory, entry->d_name, entry->d_name);
                        send(clientSocket, buff, strlen(buff), 0);
                    }
                    else
                    {
                        sprintf(buff, "<h1><a style=\"font-weight:900;color:#00008B\" href=\"http://localhost:%d/get?dir=%s/%s\">%s</a></h1>", port, directory, entry->d_name, entry->d_name);
                        send(clientSocket, buff, strlen(buff), 0);
                    }
                }
            }

            send(clientSocket, "</html>", 7, 0);
        }
        free(query);
    }
    else
    {
        // First time request
        DIR *dir = opendir(".");
        struct dirent *entry;
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
        {
            char *message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html><h1>Can't open directory</h1></html>";
            send(clientSocket, message, strlen(message), 0);
            close(clientSocket);
            return NULL;
        }

        char *message = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n<html>";
        send(clientSocket, message, strlen(message), 0);
        sprintf(buff, "<h1><a style=\"font-weight:900;color:#00008B\" href=\"http://localhost:%d\">.</a></h1>", port);
        send(clientSocket, buff, strlen(buff), 0);
        char *tmp;
        if ((tmp = getParentPath(cwd)) != NULL)
        {
            sprintf(buff, "<h1><a style=\"font-weight:900;color:#00008B\" href=\"http://localhost:%d/get?dir=%s\">..</a></h1>", port, tmp);
            send(clientSocket, buff, strlen(buff), 0);
        }

        while ((entry = readdir(dir)) != NULL)
        {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
            {
                if (entry->d_type == DT_REG)
                {
                    sprintf(buff, "<h1><i><a style=\"font-weight:100;color:#00d2ff\" href=\"http://localhost:%d/get?file=%s/%s\">%s</a></i></h1>", port, cwd, entry->d_name, entry->d_name);
                    send(clientSocket, buff, strlen(buff), 0);
                }
                else
                {
                    sprintf(buff, "<h1><a style=\"font-weight:900;color:#00008B\" href=\"http://localhost:%d/get?dir=%s/%s\">%s</a></h1>", port, cwd, entry->d_name, entry->d_name);
                    send(clientSocket, buff, strlen(buff), 0);
                }
            }
        }

        send(clientSocket, "</html>", 7, 0);
    }

    close(clientSocket);
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
    port = atoi(argv[1]);

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
    signal(SIGPIPE, signal_handler);
    while (1)
    {
        int client = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);
        if (client == -1)
        {
            perror("accept() failed");
            continue;;
        }
        printf("Client has connected: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }

    // Close
    close(serverSocket);
    printf("Socket closed\n");
    return 0;
}