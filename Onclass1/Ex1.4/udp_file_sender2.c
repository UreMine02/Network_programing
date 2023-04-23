#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUF_SIZE 20
#define PORT 9000

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in server_addr;
    char filename[256];
    char buffer[BUF_SIZE], buffer2[BUF_SIZE];
    int bytes_read, bytes_read2, bytes_sent, bytes_sent2;
    FILE *fp, *fp2;

    // create socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(1);
    }

    // setup server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // change this to receiver's IP address

    // open file for reading
    fp = fopen("send2.txt", "rb");
    if (fp == NULL) {
        perror("Failed to open file");
        exit(1);
    }

    // send filename
    bytes_sent = sendto(sock, "receive2.txt", strlen("receive2.txt"), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (bytes_sent < 0) {
        perror("Failed to send filename");
        exit(1);
    }

    // send file content
    while ((bytes_read = fread(buffer, 1, BUF_SIZE, fp)) > 0) {
        char ndsend[256];
        strcpy(ndsend, "receive2.txt_");
        strcat(ndsend, buffer);
        bytes_sent = sendto(sock, ndsend, bytes_read + 13, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if (bytes_sent < 0) {
            perror("Failed to send file content");
            exit(1);
        }
        usleep(1000000); // delay for a short time to reduce sending speed
    }
    printf("File2 sent successfully\n");
    bytes_sent = sendto(sock, "receive2.txt_receive2.txt", 25, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));

    if (bytes_sent < 0) {
        printf("Error sending end of file packet.\n");
        return -1;
    }
    // close file and socket
    fclose(fp);
    close(sock);

    return 0;
}
