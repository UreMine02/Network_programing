#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000); 

    if (connect(client, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("connect() failed");
        return 1;
    }
        
    fd_set fdread;
    char buf[256];

    while (1)
    {
        // Xóa tập fdread (Xóa tất cả các socket đã được gắn vào)
        FD_ZERO(&fdread);

        // Gắn mô tả STDIN (bàn phím) vào tập fdread
        FD_SET(STDIN_FILENO, &fdread);
        // Gắn socket vào tập fdread
        FD_SET(client, &fdread);

        // Chờ đến khi sự kiện xảy ra với các mô tả đã được gắn vào fdread
        int ret = select(client + 1, &fdread, NULL, NULL, NULL);

        // Kiểm tra sự kiện có dữ liệu từ bàn phím 
        if (FD_ISSET(STDIN_FILENO, &fdread))
        {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);
        }

        // Kiểm tra sự kiện có dữ liệu từ socket
        if (FD_ISSET(client, &fdread))
        {
            ret = recv(client, buf, sizeof(buf), 0);
            buf[ret] = 0;
            printf("%s\n", buf);
        }
    }

    close(client);

    return 0;
}