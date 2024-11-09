#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int  client(){
    // 1. 创建通信的套接字
    int clt_sock = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 连接服务器
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9999);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr.s_addr);
    int ret = connect(clt_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret == -1){
        perror("connect error");
        return 1;
    }
    // 3. 通信
    int number = 0;
    while (1){
        // 发送数据
        char buf[1024] = {0};
        sprintf(buf, "hello, server, I am client %d", number++);
        send(clt_sock, buf, strlen(buf) + 1, 0); // 这里的+1是间字符串结尾的/0也发送过去。
        // 接收数据
        memset(buf, 0, sizeof(buf));
        ret = recv(clt_sock, buf, sizeof(buf), 0);
        if (ret == -1){ // 接收失败
            perror("recv error");
            return -1;
        }else if (ret == 0){ // 服务端关闭了连接
            printf("server close\n");
            return 0;
        }else{
            printf("server say: %s\n", buf);
        }
        sleep(1);
    }

    // 4. 关闭套接字
    close(clt_sock);

    return 0;
}

//int main(){
//
//    client();
//    return 0;
//
//}