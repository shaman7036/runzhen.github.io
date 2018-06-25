#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("\n\tUsage: ./server IP PORT\n");
        return -1;
    }

    //创建套接字
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    //将套接字和IP、端口绑定
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  //每个字节都用0填充
    serv_addr.sin_family = AF_INET;  //使用IPv4地址
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);  //具体的IP地址
    serv_addr.sin_port = htons(atoi(argv[2]));  //端口

    int enable = 1;
    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int)) < 0) {
        printf("Error: set Port Reuse option failed\n");
        return -1;
    }

    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    //进入监听状态，等待用户发起请求
    listen(serv_sock, 20);

    //接收客户端请求
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    int clnt_sock;

    while (1) {
        clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        printf("%s receives request on Port %d\n", argv[0], atoi(argv[2]));
    }

    //关闭套接字
    close(clnt_sock);
    close(serv_sock);

    return 0;
}
