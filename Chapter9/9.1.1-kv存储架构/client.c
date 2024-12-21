#include <arpa/inet.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "kvstore.h"

#define SERVER_IP "127.0.0.1"

#if (ENABLE_NETWORK_SELECT == NETWORK_EPOLL)
#define SERVER_PORT 2048
#elif (ENABLE_NETWORK_SELECT == NETWORK_NTYCO)
#define SERVER_PORT 9096
#endif

#define BUFFER_SIZE 128

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 创建套接字
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 连接到服务器
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

#if 0
    char *message = NULL;
    size_t len = 0;
    while (1)
    {
        int ret = getline(&message, &len, stdin);
        if (ret == -1)
        {
            perror("getline");
            break;
        }
        if (*message == 'q')
            break;

        if (send(sockfd, message, strlen(message), 0) == -1)
        {
            perror("send");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
    }
    free(message);
#elif 0
    /*     // 发送数据
        for (int i = 0; i != 5; ++i)
        {
            char message[128];
            snprintf(message, sizeof(message), "Hello, Server! %d", i);
            if (send(sockfd, message, strlen(message), 0) == -1)
            {
                perror("send");
                close(sockfd);
                exit(EXIT_FAILURE);
            }
            sleep(1);
        }
     */

#elif 1

    char *msg[] = {"SET key setVal", "GET key", "MOD key modVal", "GET key", "SET key setVal2", "GET key", "DEL key", "GET key"};
    char *rbtree_msg[] = {"RSET key setVal", "RGET key", "RMOD key modVal", "RGET key", "RSET key setVal2", "RGET key", "RDEL key", "RGET key"};

    for (int i = 0; i < 8; i++)
    {
        if (send(sockfd, rbtree_msg[i], strlen(rbtree_msg[i]), 0) == -1)
        {
            perror("send");
            close(sockfd);
        }
        sleep(1);
    }

#endif

    close(sockfd);

    return 0;
}
