/*
 *  ┌───┐   ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┬───┐ ┌───┬───┬───┐
 *  │Esc│   │ F1│ F2│ F3│ F4│ │ F5│ F6│ F7│ F8│ │ F9│F10│F11│F12│ │P/S│S L│P/B│  ┌┐    ┌┐    ┌┐
 *  └───┘   └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┴───┘ └───┴───┴───┘  └┘    └┘    └┘
 *  ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───────┐ ┌───┬───┬───┐ ┌───┬───┬───┬───┐
 *  │~ `│! 1│@ 2│# 3│$ 4│% 5│^ 6│& 7│* 8│( 9│) 0│_ -│+ =│ BacSp │ │Ins│Hom│PUp│ │N L│ / │ * │ - │
 *  ├───┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─────┤ ├───┼───┼───┤ ├───┼───┼───┼───┤
 *  │ Tab │ Q │ W │ E │ R │ T │ Y │ U │ I │ O │ P │{ [│} ]│ | \ │ │Del│End│PDn│ │ 7 │ 8 │ 9 │   │
 *  ├─────┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴┬──┴─────┤ └───┴───┴───┘ ├───┼───┼───┤ + │
 *  │ Caps │ A │ S │ D │ F │ G │ H │ J │ K │ L │: ;│" '│ Enter  │               │ 4 │ 5 │ 6 │   │
 *  ├──────┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴─┬─┴────────┤     ┌───┐     ├───┼───┼───┼───┤
 *  │ Shift  │ Z │ X │ C │ V │ B │ N │ M │< ,│> .│? /│  Shift   │     │ ↑ │     │ 1 │ 2 │ 3 │   │
 *  ├─────┬──┴─┬─┴──┬┴───┴───┴───┴───┴───┴──┬┴───┼───┴┬────┬────┤ ┌───┼───┼───┐ ├───┴───┼───┤ E││
 *  │ Ctrl│    │Alt │         Space         │ Alt│    │    │Ctrl│ │ ← │ ↓ │ → │ │   0   │ . │←─┘│
 *  └─────┴────┴────┴───────────────────────┴────┴────┴────┴────┘ └───┴───┴───┘ └───────┴───┴───┘
 * 
 * @Author: Bssn520 Bssn520@duck.com
 * @Date: 2024-12-12 18:14:02
 * @LastEditTime: 2024-12-13 19:31:48
 * @Description: 基于 Posix API 实现 两台客户机之间的TCP通信
 * 
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFFER_LENGTH 1024
#define REMOTE_IP "10.211.55.18"
#define REMOTE_PORT 2048

struct fd_addr_t
{
    int fd;
    struct sockaddr_in addr;
};

struct fd_addr_t *get_fd(int flag, struct fd_addr_t *fd_addr, const char *ip, const int port)
{
    // flag: 1 则需要为 fd 绑定 ip port；0 则不需要
    if (!flag)
    {
        // 1.创建 fd
        fd_addr->fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd_addr->fd == -1)
        {
            perror("socket");
        }
        return NULL;
    }
    else
    {
        // 1.创建 fd
        fd_addr->fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd_addr->fd == -1)
        {
            perror("socket");
        }

        // 2. 本地 IP PORT 绑定
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_addr.s_addr = ip != NULL ? inet_addr(ip) : htonl(INADDR_ANY);
        addr.sin_port = htons(port);
        addr.sin_family = AF_INET;

        fd_addr->addr = addr;

        return fd_addr;
    }
}

int main(int argc, const char **argv)
{
    // 1.创建本地 fd
    struct fd_addr_t fd_addr;
    struct fd_addr_t *local_fd = get_fd(1, &fd_addr, NULL, 2048);
    if (bind(local_fd->fd, (struct sockaddr *)&local_fd->addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("local_fd bind");
        return -1;
    }
    if (listen(local_fd->fd, 10) == -1)
    {
        perror("local_fd listen");
        return -1;
    }

    // 3. 创建连接套接字
    int conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn_fd == -1)
    {
        perror("socket");
        return -1;
    }
    printf("conn_fd: %d\n", conn_fd);

    // 4. 对端 IP PORT 绑定
    struct fd_addr_t remote_addr;
    struct fd_addr_t *remote_fd = get_fd(1, &remote_addr, REMOTE_IP, REMOTE_PORT);

    // 5. 接收对端的连接
    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    int peer_fd = accept(local_fd->fd, (struct sockaddr *)&peer_addr, &peer_len);

    // 6. connect 到对端
    while (connect(conn_fd, (struct sockaddr *)&remote_addr.addr, sizeof(remote_addr.addr)) == -1)
    {
        printf("connect\n");
        sleep(1);
        continue;
    }

    // 7. 循环发送读取数据
    int FLAG = 1;
    while (FLAG)
    {
        char wbuf[BUFFER_LENGTH] = {0};
        for (int i = 0; i != 10; ++i)
        {
            sprintf(wbuf, "send: im ubuntu %d\n", i);
            int count = send(conn_fd, wbuf, sizeof(wbuf), 0);
            if (count == -1)
            {
                perror("send");
                return -1;
            }
            printf("Sent: %s", wbuf);
            sleep(1);

            // 接收对端数据
            char rbuf[BUFFER_LENGTH] = {0};
            int recv_count = recv(peer_fd, rbuf, sizeof(rbuf), 0);
            if (recv_count > 0)
                printf("recv: %s\n", rbuf);
            else if (recv_count == 0 | i == 10)
            {
                printf("peer disconnect...\n");
                FLAG = 0;
            }
            sleep(1);
        }
    }

    close(local_fd->fd);
    close(peer_fd);
    close(conn_fd);

    return 0;
}
