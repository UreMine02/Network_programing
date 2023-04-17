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

    // Truyen nhan du lieu
    char buf[512];
    char tmp[50];
    int res = 0;
    int ret = recv(client, buf, sizeof(buf), 0);

    buf[ret] = 0;
    
    
    char str3[512] = "";
    char *p1 = buf, *p2;
    
    /* Tạo vòng lặp để xóa hết chuỗi con */
    while((p2 = strstr(p1,"0123456789")) != NULL) { /*Tìm vị trí chuỗi con bằng hàm strstr*/
    strncat(str3,p1,p2 - p1);   /* Nối các phần không chứa chuỗi con */ 
    p1 = p2 + 10;      /* Dịch chuyển con trỏ sang vị trí tìm kiếm tiếp theo */
    res ++;
    }
    printf("So lan xuat hien cua chuoi la: %d\n", res);
    

    close(client);
    close(listener);
}