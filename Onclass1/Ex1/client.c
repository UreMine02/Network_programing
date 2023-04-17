#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char buf[512];
    char computer[64];
    int num_disk;
    

    // Nhap du lieu va dong goi
    // Nhap ten may tinh
    printf("Nhap ten may tinh: ");
    scanf("%s", computer);

    // Xoa \n trong bo dem
    getchar();

    // Chuyen vao buffer
    strcpy(buf, computer);
    int pos = strlen(computer);
    buf[pos] = 0;
    pos++;

    printf("Nhap vao so o dia: \n");
    scanf("%d", &num_disk);
    getchar();

    char disk;
    unsigned int disk_size;
    
    for(int i=0; i<num_disk; i++){
        printf("Nhap ten o dia\n");
        scanf("%c", &disk);

        printf("Nhap kich thuoc o dia\n");
        scanf("%o", &disk_size);

        getchar();

        buf[pos] = disk;
        pos++;
        memcpy(buf + pos, &disk_size, sizeof(disk_size));
        pos += sizeof(disk_size);
    }
    

    // fgets(disk, sizeof(disk), stdin);
    // getchar();

    // sprintf(buf,"%s %d %s", computer, num_disk, disk);

    printf("Buffer size: %d\n", pos);

    send(client, buf, pos, 0);
    
    close(client);
}