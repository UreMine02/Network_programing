#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/select.h>

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
    
    int clients[64];
    int num_clients = 0;
    
    char buf[256];
    char name[64][512];   
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

        // Chờ đến khi sự kiện xảy ra
        int ret = select(maxdp, &fdread, NULL, NULL, NULL);

        if (ret < 0)
        {
            perror("select() failed");
            return 1;
        }

        // Kiểm tra sự kiện có yêu cầu kết nối
        if (FD_ISSET(listener, &fdread))
        {
            int client = accept(listener, NULL, NULL);
            // printf("Ket noi moi: %d\n", client);
            clients[num_clients++] = client;
            char cmd[19] ="Enter client name:";
            send(client, cmd, strlen(cmd), 0);
            while(1){
                int ret = recv(client, buf, sizeof(buf), 0);
                if(strncmp(buf,"client id", 9)!=0){
                    char cmd[22] ="Re-Enter client name:";
                    send(client, cmd, strlen(cmd), 0);
                }else{
                    if (ret <= 0){
                    // TODO: Client đã ngắt kết nối, xóa client ra khỏi mảng
                    continue;
                    }
                    buf[ret] = 0;
                    printf("New client: %s\n", buf);
                    strncpy(name[client], buf + 11, ret - 11);
                    name[client][ret - 12] = 0;
                    break;
                }
            }
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
                for(int j = 0; j < num_clients; j++){
                    if(j != i){
                        send(clients[j], name[clients[i]], strlen(name[clients[i]]) , 0);
                        send(clients[j], ": ",2 ,0);
                        send(clients[j], buf, strlen(buf) , 0);
                    }
                }
            }
    }

    close(listener);    

    return 0;
}