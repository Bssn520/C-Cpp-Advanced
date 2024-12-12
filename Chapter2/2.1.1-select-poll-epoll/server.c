#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

void *client_thread(void *args)
{
    int clientfd = *(int *)args;
    while (1)
    {
        char buff[128] = {0};
        int count = recv(clientfd, buff, 128, 0);
        if (count == 0)
            break;
        else if (count == -1)
        {
            perror("recv");
            break; // 发生错误
        }
        send(clientfd, buff, count, 0);
        printf("clientfd: %d, count: %d, buffer: %s\n", clientfd, count, buff);
    }
    close(clientfd);

    return NULL;
}

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(2048);

    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind");
        return -1;
    }
    listen(sockfd, 10);

#if 0 // 多线程实现多路io
    while (1)
    {
        struct sockaddr_in clientaddr;
        socklen_t len = sizeof(clientaddr);
        int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, &clientfd);
    }
#elif 0 // select 实现io多路复用
    fd_set rfds, rset; // rfds可读文件描述符；rset可读文件描述符集合
    FD_ZERO(&rfds); // 清空 rfds
    FD_SET(sockfd, &rfds); // 绑定

    int maxfd = sockfd;

    printf("loop\n");
    while (1)
    {
        rset = rfds;

        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        // 如果 i 在 rset 中，FD_ISSET 返回 1，表示该文件描述符已经准备好进行 io 操作；否则，返回 0
        if (FD_ISSET(sockfd, &rset))
        {
            struct sockaddr_in clientaddr;
            socklen_t len = sizeof(clientaddr);

            int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);

            printf("sockfd: %d\n", clientfd);

            FD_SET(clientfd, &rfds);

            maxfd = clientfd;
        }

        for (int i = sockfd + 1; i < maxfd + 1; ++i)
        {
            if (FD_ISSET(i, &rset))
            {

                char buff[128] = {0};
                int count = recv(i, buff, 128, 0);
                if (count == 0)
                {
                    printf("disconnected\n");
                    FD_CLR(i, &rfds);
                    close(i);
                    break;
                }
                send(i, buff, count, 0);
                printf("clientfd: %d, count: %d, buffer: %s\n", i, count, buff);
            }
        }
    }

#elif 0 // poll 实现io多路复用
    struct pollfd fds[1024] = {0};
    fds[sockfd].fd = sockfd;
    fds[sockfd].events = POLLIN;

    int maxfd = sockfd;

    while (1)
    {
        int nready = poll(fds, maxfd + 1, -1);
        if (fds[sockfd].revents & POLLIN)
        {
            struct sockaddr_in clientaddr;
            socklen_t len = sizeof(clientaddr);

            int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);

            printf("sockfd: %d\n", clientfd);
            fds[clientfd].fd = clientfd;
            fds[clientfd].events = POLLIN;

            maxfd = clientfd;
        }

        for (int i = sockfd + 1; i < maxfd + 1; ++i) // i = sockfd + 1 是因为在上面 sockfd 已经处理过了
        {
            if (fds[i].revents & POLLIN)
            {
                char buff[128] = {0};
                int count = recv(i, buff, 128, 0);
                if (count == 0)
                {
                    printf("disconnected\n");:q


                    // FD_CLR(i, &rfds);
                    fds[i].fd = -1;
                    fds[i].events = 0;

                    close(i);

                    break;
                }
                send(i, buff, count, 0);
                printf("clientfd: %d, count: %d, buffer: %s\n", i, count, buff);
            }
        }
    };

#elif 1 // epoll 实现 io 多路复用
    int epfd = epoll_create(1);
    struct epoll_event ev;
    ev.data.fd = sockfd;
    ev.events = EPOLLIN;

    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev); // 将 sockfd 添加进epfd

    struct epoll_event events[1024] = {0};

    while (1)
    {
        int nready = epoll_wait(epfd, events, 1024, -1);

        for (int i = 0; i < nready; ++i)
        {
            int connfd = events[i].data.fd;
            if (sockfd == connfd)
            {

                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);

                int clientfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);

                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
                printf("clientfd: %d\n", clientfd);
            }
            else if (events[i].events & EPOLLIN)
            {

                char buff[128] = {0};
                int count = recv(connfd, buff, 128, 0);
                if (count == 0)
                {
                    printf("disconnected\n");

                    epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
                    close(i);

                    break;
                }
                send(connfd, buff, count, 0);
                printf("clientfd: %d, count: %d, buffer: %s\n", i, count, buff);
            }
        }
    }

#endif

    close(sockfd);

    return 0;
}
