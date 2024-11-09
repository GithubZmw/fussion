#include "server.h"
#include <pthread.h>


/**
 * socket的最基本用法演示，用于给初学者学习socket的使用流程
 * @return
 */
int socket() {
    /**
     * 1. 创建套接字，
     * AF_INET:表示使用IPv4地址,使用AF_INET6表示使用IPv6地址
     * SOCK_STREAM：表示使用传输层协议中的流式传输协议
     * IPPROTO_TCP：表示使用流式传输协议中的TCP协议
     */
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serv_sock == -1) {
        perror("socket error");
        return -1;
    }

    /**
     * 2. 将套接字和IP、端口绑定。做这一步是因为客户端要想连接服务器，需要首先知道服务器的IP地址和端口号，因此服务端需要首先绑定IP和端口
     * ① 绑定IP和端口使用 bind 函数，该函数接收结构体sockaddr，里面存储了IP和端口，但是这个结构体的赋值比较麻烦，因此使用一个更容易程序员
     * 操作的结构体sockaddr_in，后面直接强转即可得到sockaddr
     * ② inet_addr 函数将点分十进制的IP地址转换为网络字节序的IP地址（也就是大端存储的形式）
     * ③ htons 函数将端口号转换为网络字节序的端口号（也就是大端存储的形式）
     * ④ sockaddr_in 结构体赋值IP和端口的方法
     *      serv_addr.sin_family = AF_INET;  指定IP地址的类型，AF_INET表示使用IPv4地址
     *      serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  指定服务器具体的IP地址（网络字节序）
     *      serv_addr.sin_port = htons(9999);  指定端口号（网络字节序）
     */
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  //具体的IP地址
    serv_addr.sin_port = htons(9999);  //端口
    int ret = bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));// 绑定IP和端口
    if (ret == -1) {
        perror("bind error");
        return -1;
    }

    /**
     * 3. 进入监听状态，等待用户发起请求
     * listen 函数将套接字设置为监听状态，等待客户端的连接，参数1是套接字，参数2是监听队列的长度，表示最多有多少个客户端可以排队等待连接
     * 这里的参数2最大为128，如果设置的值比128大，那么他也使用128
     */
    ret = listen(serv_sock, 128);
    if (ret == -1) {
        perror("listen error");
        return -1;
    }

    /**
     * 4. 接收客户端请求,这里线程阻塞，等待客户端的链接
     * accept 函数接收客户端的连接，参数1是服务器的套接字，参数2是客户端的IP和端口（这个结构体知识用来存储监听到的客户端IP和端口信息），参数3是客户端的IP和端口的大小
     * 如果客户端连接成功，那么返回一个新的套接字，这个套接字专门用来和客户端进行通信，而参数1这个套接字（服务器套接字）仍然用来监听其他客户端的连接
     */
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock = accept(serv_sock, (struct sockaddr *) &clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1) {
        perror("accept error");
        return -1;
    }
    // 打印客户端的IP地址和端口号
    printf("client IP: %s, port: %d\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

    /**
     * 5.1 接收客户端发送的数据
     * 为了更好的理解文件描述符的作用，这里对其进行进一步的解释。事实上，每个文件描述符对应于内存中的两个缓冲区，读缓冲区和写缓冲区
     * 服务器会维护一个监听文件描述符和若干个通信文件描述符，其中每个已经与服务器连接的客户端对应于一个通信文件描述符
     * 当客户端调用connect函数连接服务器时，他实际上是将连接请求写入了自己的写缓冲区，然后内核会自动将请求发送到服务器的读缓冲区
     * 此时服务器的accept函数就会将请求从读缓冲区中读取出来，然后创建一个新的通信文件描述符，这个通信文件描述符对应于客户端的IP地址和端口号
     * 当客户端要给服务器发送消息时，它将消息写入自己文件描述符对应的写缓冲区，然后内核自动将消息发送到服务器中用于与该客户端通讯的文件描述符对应的读缓冲区
     * 接着，服务器从这个读缓冲区中将消息读取出来，从而得到了客户端的通信消息。服务器与客户端通信的过程和上述过程基本相同。这里不再赘述。
     * 注意：
     *  1. 需要注意的是，无论是客户端还是服务端，它们在调用read(recv)或write(send)时，并没有将消息直接通过指定的协议发送给对方，只是简单的将消息
     *  写入到了自己文件描述符相应的缓冲区之中，消息在网络中的传输其实是内核自动进行维护的。
     *  2. 当缓冲区中没有数据的时候，调用read(recv)或write(send)函数会使线程进入阻塞的状态，只有监听到缓冲区中有数据的时候，线程才会解除阻塞状态，
     *  然后从缓冲区中读取数据。
     */
    while (true) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int len = read(clnt_sock, buffer, sizeof(buffer));
        if (len > 0) {
            // 5.2 如果收到了客户端发来的消息,向客户端打招呼
            printf("client request: %s\n", buffer);
            char str[] = "hello client";
            write(clnt_sock, str, sizeof(str));
        } else if (len == 0) {
            printf("client closed\n");
            break;
        } else {
            perror("read error");
            break;
        }
    }

    // 6. 关闭套接字
    close(clnt_sock);
    close(serv_sock);

    return 0;
}



struct SockInfo
{
    int fd;                      // 通信
    pthread_t tid;               // 线程ID
    struct sockaddr_in addr;     // 地址信息
};

struct SockInfo infos[128];


void* working(void* arg)
{
    while(1)
    {
        struct SockInfo* info = (struct SockInfo*)arg;
        // 接收数据
        char buf[1024];
        int ret = read(info->fd, buf, sizeof(buf));
        if(ret == 0)
        {
            printf("客户端已经关闭连接...\n");
            info->fd = -1;
            break;
        }
        else if(ret == -1)
        {
            printf("接收数据失败...\n");
            info->fd = -1;
            break;
        }
        else
        {
            write(info->fd, buf, strlen(buf)+1);
        }
    }
    return NULL;
}


/**
 * 多线程模式的socket服务端实现，可以同时与多个客户端进行通信
 * @return
 */
int socket2(){
    // 1. 创建用于监听的套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        perror("socket");
        exit(0);
    }

    // 2. 绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;          // ipv4
    addr.sin_port = htons(9999);        // 字节序应该是网络字节序
    addr.sin_addr.s_addr =  INADDR_ANY; // == 0, 获取IP的操作交给了内核
    int ret = bind(fd, (struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1)
    {
        perror("bind");
        exit(0);
    }

    // 3.设置监听
    ret = listen(fd, 100);
    if(ret == -1)
    {
        perror("listen");
        exit(0);
    }

    // 4. 等待, 接受连接请求
    socklen_t len = sizeof(struct sockaddr);

    // 数据初始化
    int max = sizeof(infos) / sizeof(infos[0]);
    for(int i=0; i<max; ++i)
    {
        bzero(&infos[i], sizeof(infos[i]));
        infos[i].fd = -1;
        infos[i].tid = -1;
    }

    // 父进程监听, 子进程通信
    while(true)
    {
        // 创建子线程
        struct SockInfo* pinfo;
        for(int i=0; i<max; ++i)
        {
            if(infos[i].fd == -1)
            {
                pinfo = &infos[i];
                break;
            }
            if(i == max-1)
            {
                sleep(1);
                i--;
            }
        }

        int connfd = accept(fd, (struct sockaddr*)&pinfo->addr, &len);
        printf("parent thread, connfd: %d\n", connfd);
        if(connfd == -1)
        {
            perror("accept");
            exit(0);
        }
        pinfo->fd = connfd;
        pthread_create(&pinfo->tid, NULL, working, pinfo);
        pthread_detach(pinfo->tid);
    }
    // 释放资源
    close(fd);  // 监听
    return 0;
}

//int main()
//{
//    socket2();
//    return 0;
//}

