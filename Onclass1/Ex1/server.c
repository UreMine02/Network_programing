#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

char str[256];
char cmd[16], port[5], tmp[50], IPaddr[50];

    
int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
    printf("Waiting for connection from client: \n");
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(addr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen);
    printf("Client IP: %s:%d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    if (client == -1){
        perror("accept() failed");
        return 1;
    }
    printf("New client connected: %d\n", client);

    // char computer[128];
    // int num_disk;
    // char cnum_disk[3];
    // char disk[128];
    // char buf[512];
    // int tmp;
    // int tmp2 = 1;  

    // int ret = recv(client, buf, sizeof(buf), 0);

    // for(int i=0; i<sizeof(buf); i++){
    //     if(isdigit(buf[i])) tmp = i+1;
    //     break;
    // }

    // for(int i=0; i<sizeof(buf); i++){
    //     if(isdigit(buf[i])){
    //         for(int j=i+1; i<sizeof(buf); i++){
    //             if(isdigit(buf[j])){
    //                 tmp2 ++;
    //             }else break;
    //         }
    //     }
    // }

    // memcpy(computer, buf, tmp);
    // computer[tmp] = 0;
    // memcpy(disk, buf + tmp2 + tmp, ret - tmp2 - tmp);
    // disk[ret - tmp2 - tmp] = 0;
    // memcpy(cnum_disk, buf + tmp, tmp2);
    // cnum_disk[tmp2] = 0;
    // num_disk = atoi(cnum_disk);

    // printf("Ten may tinh");
    // printf("%s\n", computer);
    // printf("So o dia");
    // printf("%d\n", num_disk);
    // printf("%s", disk);

    char buf[512];
    int ret = recv(client, buf, sizeof(buf), 0);
    
    buf[ret] = 0;

    int pos = 0;

    // Tach du lieu tu buffer
    char computer[64];
    strcpy(computer, buf);

    pos += strlen(computer) + 1;

    printf("Computer name: %s\n", computer);


    int num_drives = (ret - pos) / 4;
    for (int i = 0; i < num_drives; i++)
    {
        char drive_letter;
        int drive_size;

        drive_letter = buf[pos];
        pos++;

        memcpy(&drive_size, buf + pos, sizeof(drive_size));
        pos += sizeof(drive_size);

        printf("%c: %dGB\n", drive_letter, drive_size);
    }

    close(client);
    close(listener);
}