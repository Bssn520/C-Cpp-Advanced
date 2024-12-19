#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "kvstore.h"

int set_event(int fd, int event, int flag);
int accept_cb(int listern_fd);
int recv_cb(int connected_fd);
int send_cb(int connected_fd);

int epfd = 0;
connection_t connection_list[1024] = {0};

int epoll_entry(void)
{
    // 1. 初始化监听文件描述符并开启监听
    int listern_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(2048);

    if (bind(listern_fd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind");
        return -1;
    }
    listen(listern_fd, 10);
    connection_list[listern_fd].fd = listern_fd;
    connection_list[listern_fd].recv_t.accept_callback = accept_cb;

    // 2. 初始化 epoll 实例，并把监听文件描述符添加进去
    epfd = epoll_create(1);
    // 设置 listern_fd 在 epoll 中的状态
    set_event(listern_fd, EPOLLIN, 1);

    while (1)
    {
        // 创建一个 epoll event 数组，用来存储所有文件描述符的事件
        struct epoll_event events[1024] = {0};

        // 3.epoll_wait 拿到就绪状态的文件描述符
        int nready = epoll_wait(epfd, events, 1024, -1);

        // 4. 对拿到的文件描述符进行遍历，目的是处理每一个文件描述符
        for (int i = 0; i < nready; ++i)
        {
            int connected_fd = events[i].data.fd; // 每轮遍历拿到的描述符为 connected_fd

            if (events[i].events & EPOLLIN) // recv 读取该文件描述符读缓冲区的数据
            {
                int count = connection_list[connected_fd].recv_t.recv_callback(connected_fd);
                // printf("recv <-- buffer: %s\n", connection_list[connected_fd].rbuffer);
            }
            // 6. 如果该连接写缓冲区有数据，我们要处理这些数据
            else if (events[i].events & EPOLLOUT) // send 发送该文件描述符读缓冲区的数据
            {
                // printf("send --> buffer: \n%s\n", connection_list[connected_fd].wbuffer);
                int count = connection_list[connected_fd].send_callback(connected_fd);
            }
        }
    }
    close(listern_fd);

    return 0;
}

int set_event(int fd, int event, int flag)
{
    // flag: 1 为新增文件描述符；0 为修改已存在文件描述符
    if (flag)
    {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

        return 0;
    }
    else
    {
        struct epoll_event ev;
        ev.events = event;
        ev.data.fd = fd;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);

        return 0;
    }

    return -1;
}

// 当 listern fd EPOLLIN 时
int accept_cb(int listern_fd)
{
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);

    int clientfd = accept(listern_fd, (struct sockaddr *)&clientaddr, &len);
    if (clientfd < 0)
        return -1;

    // 设置新增 clientfd 状态
    set_event(clientfd, EPOLLIN, 1);

    connection_list[clientfd].fd = clientfd;
    memset(connection_list[clientfd].rbuffer, 0, BUFFER_LENGTH);
    connection_list[clientfd].rlength = 0;
    memset(connection_list[clientfd].wbuffer, 0, BUFFER_LENGTH);
    connection_list[clientfd].wlength = 0;
    connection_list[clientfd].recv_t.recv_callback = recv_cb;
    connection_list[clientfd].send_callback = send_cb;

    return clientfd;
}

// 当 client fd EPOLLIN 时
int recv_cb(int connected_fd)
{
    // recv 读取该文件描述符读缓冲区的数据
    char *buffer = connection_list[connected_fd].rbuffer;
    int index = connection_list[connected_fd].rlength;
    int count = recv(connected_fd, buffer, BUFFER_LENGTH, 0);
    if (count == 0)
    {
        // printf("disconnected\n");

        epoll_ctl(epfd, EPOLL_CTL_DEL, connected_fd, NULL);
        close(connected_fd);

        return -1;
    }
    connection_list[connected_fd].rlength += count;

    // 发送完成后修改 connected_fd 状态以待下次接收
    set_event(connected_fd, EPOLLOUT, 0);

    kvstore_request(&connection_list[connected_fd]);
    connection_list[connected_fd].wlength = strlen(connection_list[connected_fd].wbuffer);

    return count;
}

// 当 client fd EPOLLOUT 时
int send_cb(int connected_fd)
{
    char *buffer = connection_list[connected_fd].wbuffer;
    int index = connection_list[connected_fd].wlength;

    int count = send(connected_fd, buffer, index, 0);

    // 发送完成后修改 connected_fd 状态以待下次发送
    set_event(connected_fd, EPOLLIN, 0);

    return count;
}
