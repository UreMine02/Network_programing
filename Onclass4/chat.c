#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct
{
    char nickname[BUFFER_SIZE];
    int socket;
} Client;

typedef struct
{
    int index;
    Client *clients;
} ThreadData;

void send_response(int client_socket, const char *response)
{
    if (send(client_socket, response, strlen(response), 0) < 0)
    {
        perror("Send error");
    }
}

int is_valid_nickname(const char *nickname, Client *clients, int client_count)
{
    for (int i = 0; i < client_count; i++)
    {
        if (strcmp(nickname, clients[i].nickname) == 0)
        {
            return 0; // Nickname already in use
        }
    }
    return 1;
}

int find_client_by_nickname(const char *nickname, Client *clients, int client_count)
{
    for (int i = 0; i < client_count; i++)
    {
        if (strcmp(nickname, clients[i].nickname) == 0)
        {
            return i; // Client found
        }
    }
    return -1; // Client not found
}

void *client_thread(void *data)
{
    ThreadData *thread_data = (ThreadData *)data;
    int client_index = thread_data->index;
    Client *clients = thread_data->clients;
    int number_client = 0;

    char buffer[BUFFER_SIZE];
    int client_socket = clients[client_index].socket;

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        // Receive message from the client
        if (recv(client_socket, buffer, sizeof(buffer), 0) <= 0)
        {
            perror("Receive error");
            break;
        }

        printf("Received message from client: %s\n", buffer);

        // Process the received message and send a response
        char command[BUFFER_SIZE];
        char parameter[BUFFER_SIZE];
        sscanf(buffer, "%s %[^\n]", command, parameter);

        if (strcmp(command, "JOIN") == 0)
        {
            // Process JOIN command
            if (!is_valid_nickname(parameter, clients, client_index))
            {
                send_response(client_socket, "200 NICKNAME IN USE\n");
            }
            else if (strlen(parameter) == 0 || strlen(parameter) > BUFFER_SIZE - 1)
            {
                send_response(client_socket, "201 INVALID NICK NAME\n");
            }
            else
            {
                strcpy(clients[client_index].nickname, parameter);
                number_client++;
                send_response(client_socket, "100 OK\n");
            }
        }
        else if (strcmp(command, "MSG") == 0)
        {
            // Process MSG command
            if (strlen(parameter) == 0 || strlen(parameter) > BUFFER_SIZE - 1)
            {
                send_response(client_socket, "999 UNKNOWN ERROR\n");
            }
            else
            {
                printf("So nguoi: %d", number_client);
                for (int i = 0; i < number_client + 2; i++)
                {
                    if(i != client_index) {
                        printf("Nguoi nhan la: %s", clients[i].nickname);
                        send_response(clients[i].socket, parameter);
                    }
                }
                send_response(client_socket, "100 OK\n");
            }
        }
        else if (strcmp(command, "OP") == 0)
        {
            // Process OP command
            if (client_index != 0)
            {
                send_response(client_socket, "203 DENIED\n");
            }
            else
            {
                int op_index = find_client_by_nickname(parameter, clients, client_index);
                if (op_index == -1)
                {
                    send_response(client_socket, "202 UNKNOWN NICKNAME\n");
                }
                else
                {
                    // Handle OP command
                    // ...
                    send_response(client_socket, "100 OK\n");
                }
            }
        }
        else if (strcmp(command, "KICK") == 0)
        {
            // Process KICK command
            if (client_index != 0)
            {
                send_response(client_socket, "203 DENIED\n");
            }
            else
            {
                int kicked_index = find_client_by_nickname(parameter, clients, client_index);
                if (kicked_index == -1)
                {
                    send_response(client_socket, "202 UNKNOWN NICKNAME\n");
                }
                else
                {
                    // Handle KICK command
                    // ...
                    send_response(client_socket, "100 OK\n");
                    number_client--;
                }
            }
        }
        else if (strcmp(command, "TOPIC") == 0)
        {
            // Process TOPIC command
            if (client_index != 0)
            {
                send_response(client_socket, "203 DENIED\n");
            }
            else
            {
                // Handle TOPIC command
                // ...
                send_response(client_socket, "100 OK\n");
            }
        }
        else if (strcmp(command, "PMSG") == 0)
        {
            // Process PMSG command
            char recipient[BUFFER_SIZE];
            char message[BUFFER_SIZE];
            sscanf(parameter, "%s %[^\n]", recipient, message);
            int recipient_index = find_client_by_nickname(recipient, clients, client_index);
            if (recipient_index == -1)
            {
                send_response(client_socket, "202 UNKNOWN NICKNAME\n");
            }
            else if (strlen(message) == 0 || strlen(message) > BUFFER_SIZE - 1)
            {
                send_response(client_socket, "999 UNKNOWN ERROR\n");
            }
            else
            {
                send_response(clients[recipient_index].socket, message);
                send_response(client_socket, "100 OK\n");
            }
        }
        else if (strcmp(command, "QUIT") == 0)
        {
            // Process QUIT command
            send_response(client_socket, "100 OK\n");
            break;
        }
        else
        {
            send_response(client_socket, "999 UNKNOWN ERROR\n");
        }
    }

    // Close the client socket
    close(client_socket);

    printf("Client disconnected: %s\n", clients[client_index].nickname);

    return NULL;
}

int main()
{
    int server_socket, client_count = 0;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length;

    Client clients[MAX_CLIENTS];

    // Create server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Set server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9000);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind the server socket to a specific address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        perror("Binding error");
        exit(EXIT_FAILURE);
    }

    // Start listening for client connections
    if (listen(server_socket, MAX_CLIENTS) < 0)
    {
        perror("Listening error");
        exit(EXIT_FAILURE);
    }

    printf("Server started. Waiting for clients...\n");

    while (1)
    {
        // Accept a new client connection
        client_address_length = sizeof(client_address);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_length);
        if (client_socket < 0)
        {
            perror("Accept error");
            exit(EXIT_FAILURE);
        }

        printf("Client connected: %s\n", inet_ntoa(client_address.sin_addr));

        clients[client_count].socket = client_socket;

        // Create a new thread for the client
        pthread_t thread;
        ThreadData thread_data = {client_count, clients};
        if (pthread_create(&thread, NULL, client_thread, (void *)&thread_data) != 0)
        {
            perror("Thread creation error");
            exit(EXIT_FAILURE);
        }

        // Detach the thread (so its resources can be automatically released when it exits)
        pthread_detach(thread);

        client_count++;
    }

    // Close the server socket
    close(server_socket);

    return 0;
}