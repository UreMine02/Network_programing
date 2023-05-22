#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>
#include <string.h>

char* chuanHoaXau(char* xau) {
    int i, j = 0;
    int length = strlen(xau);
    
    // Loại bỏ khoảng trắng thừa ở đầu xâu
    while (xau[j] == ' ') {
        j++;
    }
    if (j > 0) {
        for (i = 0; i < length - j; i++) {
            xau[i] = xau[i + j];
        }
        xau[i] = '\0';
        length -= j;
    }
    
    // Loại bỏ khoảng trắng thừa ở cuối xâu
    i = length - 1;
    while (xau[i] == ' ') {
        xau[i] = '\0';
        i--;
    }
    
    // Chuẩn hóa khoảng trắng giữa các từ
    for (i = 0; i < length; i++) {
        if (xau[i] == ' ' && xau[i + 1] == ' ') {
            int j = i;
            while (xau[j] != '\0') {
                xau[j] = xau[j + 1];
                j++;
            }
            i--;
            length--;
        }
    }
    
    // Viết hoa chữ cái đầu mỗi từ
    for (i = 0; i < length; i++) {
        if (i == 0 || xau[i - 1] == ' ') {
            if (xau[i] >= 'a' && xau[i] <= 'z') {
                xau[i] = xau[i] - 32;
            }
        } else {
            if (xau[i] >= 'A' && xau[i] <= 'Z') {
                xau[i] = xau[i] + 32;
            }
        }
    }
    return xau;
}
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

    fd_set fdread;
    char cmd[256];
    char res[512];
    int clients[64];
    int num_clients = 0;
    
    char buf[256];

    struct timeval tv;
    
    while (1)
    {
        // Xóa tất cả socket trong tập fdread
        FD_ZERO(&fdread);

        // Thêm socket listener vào tập fdread
        FD_SET(listener, &fdread);
        int maxdp = listener + 1;

        // Thêm các socket client vào tập fdread
        for (int i = 0; i < num_clients; i++)
        {
            FD_SET(clients[i], &fdread);
            if (maxdp < clients[i] + 1)
                maxdp = clients[i] + 1;
        }

        // Thiet lap thoi gian cho
        // tv.tv_sec = 5;
        // tv.tv_usec = 0;

        // Chờ đến khi sự kiện xảy ra
        int ret = select(maxdp, &fdread, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        if (ret == 0)
        {
            printf("Timed out!!!\n");
            continue;
        }

        // Kiểm tra sự kiện có yêu cầu kết nối
        if (FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            printf("Ket noi moi: %d\n", client);
            clients[num_clients++] = client;
            char l1[64] = "Xin chao. Hien co";
            char l2[64] = "Ket noi";
            sprintf(cmd, "%s %d %s",l1, num_clients, l2);
            send(client, cmd, sizeof(cmd), 0);
            printf("%s", cmd);
        }

        // Kiểm tra sự kiện có dữ liệu truyền đến socket client
        for (int i = 0; i < num_clients; i++)
            if (FD_ISSET(clients[i], &fdread))
            {
                ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    // TODO: Client đã ngắt kết nối, xóa client ra khỏi mảng
                    continue;
                }
                buf[ret] = 0;
                if(strcmp(buf,"exit")==0){
                    close(clients[i]);
                    num_clients--;
                    FD_CLR(i, &fdread);
                }
                // printf("Du lieu nhan tu %d: %s\n", clients[i], buf);
                strcpy(res,chuanHoaXau(buf));
                send(clients[i], res, sizeof(res), 0);
            }
    }

    close(listener);    

    return 0;
}