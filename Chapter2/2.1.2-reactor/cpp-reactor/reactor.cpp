#include "reactor.hpp"
#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <ctime>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/epoll.h> // epoll
#include <sys/socket.h> // socket
#include <unistd.h> //close函数

#define BUFFER_LENGTH 1024

int epfd;
connection_item connection_list[1024];

// 向epoll中添加文件描述符 或 设置epoll中的某个文件描述符的状态
int set_event(int fd, int event, int flag);
int accept_cb(int listern_fd);
int recv_cb(int connected_fd);
int send_cb(int connected_fd);

int main(int argc, const char **argv)
{
    // 1. 创建一个 tcp socket
    int listern_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in sock_addr;
    sock_addr.sin_addr.s_addr = htons(INADDR_ANY);
    sock_addr.sin_port = htons(2048);
    sock_addr.sin_family = AF_INET;

    // 2. 配置 socket 的地址和端口
    if (bind(listern_fd, (sockaddr *)&sock_addr, sizeof(sock_addr)) == -1)
    {
        std::cout << "bind" << std::endl;
        return -1;
    }

    // 3. 开始监听 socket
    if (listen(listern_fd, 10) == -1)
    {
        std::cout << "listern" << std::endl;
        return -1;
    }

    epfd = epoll_create(1);

    epoll_event events[1024];

    set_event(listern_fd, EPOLL_CTL_ADD, 1);


    return 0;
}

int set_event(int fd, int event, int flag)
{
    // flag: true 为添加文件描述符到 epoll，false 为修改已存在文件描述符
    if (flag)
    {
        epoll_event ev;
        ev.data.fd = fd;
        ev.events = event;
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

        return 0;
    }
    else
    {
        epoll_event ev;
        ev.data.fd = fd;
        ev.events = event;
        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);

        return 0;
    }
}

int accept_cb(int listern_fd)
{
    sockaddr_in clientaddr;
    socklen_t len;

    // 1. 新建 client 连接
    int clientfd = accept(listern_fd, (sockaddr *)&clientaddr, &len);
    if (clientfd < 0)
    {
        std::cout << "client" << std::endl;
        return -1;
    }

    // 2. 将 clientfd 添加到 epoll 并初始化
    set_event(clientfd, EPOLLIN, 1);

    connection_list[clientfd].fd = clientfd;
    std::fill(connection_list[clientfd].rbuf.begin(), connection_list[clientfd].rbuf.end(), 0);
    connection_list[clientfd].rlength = 0;
    std::fill(connection_list[clientfd].wbuf.begin(), connection_list[clientfd].wbuf.end(), 0);
    connection_list[clientfd].wlength = 0;
    connection_list[clientfd].recv_t.recv_callback = recv_cb;
    connection_list[clientfd].send_callback = send_cb;

    return clientfd;
}
int recv_cb(int connected_fd)
{
    std::string buf = connection_list[connected_fd].rbuf;
    int index = connection_list[connected_fd].rlength;
    int count = recv(connected_fd, buf.data() + index, BUFFER_LENGTH - index, 0);
}
int send_cb(int connected_fd)
{
}