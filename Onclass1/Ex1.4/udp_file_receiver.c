#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 4096
#define PORT 9000
int sock;
struct sockaddr_in server_addr, client_addr;
socklen_t client_addr_len;
// char filenames[256];
char buffer[BUF_SIZE];
// int bytes_received, bytes_written;
FILE *f1, *f2;

int main(int argc, char *argv[]) {
    char *filenames1;
    char *filenames2;
    // create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(1);
    }
    // setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    // bind socket to port
    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind socket");
        exit(1);
    }
    printf("Listening on port %d...\n", PORT);
    while (1) {
        usleep(1000000);
        client_addr_len = sizeof(client_addr);
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if(!strchr(buffer, '_')) {
            if(!filenames1) {
                char *fn1 = (char*) malloc(strlen(buffer)+1);
                memcpy(fn1, buffer, strlen(buffer)+1);
                filenames1 =  fn1;
                f1 = fopen(filenames1, "wb");
                if (f1 == NULL) perror("Failed to open file1");
            } 
            else if(!filenames2) {
                char *fn2 = (char*) malloc(strlen(buffer)+1);
                memcpy(fn2, buffer, strlen(buffer)+1);
                filenames2 =  fn2;
                f2 = fopen(filenames2, "wb");
                if (f2 == NULL) perror("Failed to open file2");
            }
        } else {
            char * token = strtok(buffer, "_");
            if(strcmp(filenames1, token)){
                int bytes_received2 = bytes_received;
                token = strtok(NULL, "_");
                char * buffers2 = token;
                if (!strcmp(buffers2, filenames2)) {
                    printf("Đóng file: %s\n", filenames2);
                    fclose(f2);
                }
                else {
                    bytes_received2 = bytes_received2 - strlen(filenames2) - 1;
                    if (bytes_received2 < 0) {
                        perror("Failed to receive file content");
                    }
                    int bytes_written2 = fwrite(buffers2, 1, bytes_received2, f2);
                    printf("File: %s Dữ liệu: %s\n", filenames2, buffers2);
                    if (bytes_written2 < bytes_received2) {
                        perror("Failed to write file content");
                    }
                }
            } else {
                int bytes_received1 = bytes_received;
                token = strtok(NULL, "_");
                char * buffers1 = token;
                if (!strcmp(buffers1, filenames1)) {
                    printf("Đóng file: %s\n", filenames1);
                    fclose(f1);
                }
                else {
                    bytes_received1 = bytes_received1 - strlen(filenames1) - 1;
                    if (bytes_received1 < 0) {
                        perror("Failed to receive file content");
                    }
                    int bytes_written1 = fwrite(buffers1, 1, bytes_received1, f1);
                    printf("File: %s Dữ liệu: %s\n", filenames1, buffers1);
                    if (bytes_written1 < bytes_received1) {
                        perror("Failed to write file content");
                    }
                }
            }
        }
    }
    printf("File received successfully\n");
    close(sock);

    return 0;
}
