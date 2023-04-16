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

    char arr[4][100]; 
    printf("Vui long nhap thong tin: \n");
    printf("MSSV: ");
    fgets(arr[0], sizeof(arr[0]), stdin);
    printf("Ho va Ten: ");
    fgets(arr[1], sizeof(arr[1]), stdin);
    printf("Ngay sinh: ");
    fgets(arr[2], sizeof(arr[2]), stdin);
    printf("CPA: ");
    fgets(arr[3], sizeof(arr[3]), stdin);
    // Gui mang sang server
    send(client, arr, sizeof(arr), 0);
    
    close(client);
}