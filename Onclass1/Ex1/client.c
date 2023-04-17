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
    char computer[128];
    int num_disk;
    char disk[128];
    

    // Nhap du lieu va dong goi
    printf("Nhap ten may tinh: \n");
    fgets(computer, sizeof(computer), stdin);

    printf("So o dia: \n");
    scanf("%d", &num_disk);
    // getchar();
    sprintf(buf,"%s %d", computer, num_disk);
    for(int i = 0; i < num_disk; i++) {
        char disk[10];
        fgets(disk, sizeof(disk), stdin);
        // fflush;
        // sprintf(buf,"%s %s",buf, disk);
        strcat(buf, disk);
    }
    

    // fgets(disk, sizeof(disk), stdin);
    // getchar();

    // sprintf(buf,"%s %d %s", computer, num_disk, disk);
    send(client, buf, sizeof(buf), 0);

    
    close(client);
}