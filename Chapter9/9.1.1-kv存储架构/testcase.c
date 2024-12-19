#include <arpa/inet.h>
#include <bits/types/struct_timeval.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "kvstore.h"

#define MSG_MAX_LENGTH 512

#define TIME_SUB_MS(tv1, tv2) ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)

int send_msg(int connfd, char *msg, int length)
{
    int res = send(connfd, msg, length, 0);
    if (res < 0)
    {
        perror("send");
        exit(1);
    }
    return res;
}

int recv_msg(int connfd, char *msg, int length)
{
    int res = recv(connfd, msg, length, 0);
    if (res < 0)
    {
        perror("recv");
        exit(1);
    }
    return res;
}

void equals(char *pattern, char *result, char *casename)
{
    if (strcmp(pattern, result) == 0)
    {
#ifdef DEBUG
        printf("==> PASS --> %s\n", casename);
#endif // DEBUG
    }
    else
    {
        printf("==> FAILED --> %s, '%s' != '%s'\n", casename, pattern, result);
    }
}

void test_case(int connfd, char *msg, char *pattern, char *casename)
{
    if (!msg || !pattern || !casename)
        return;

    send_msg(connfd, msg, strlen(msg));

    char result[MSG_MAX_LENGTH] = {0};
    recv_msg(connfd, result, MSG_MAX_LENGTH);

    equals(pattern, result, casename);
}

void array_testcase(int connfd)
{
    test_case(connfd, "SET Name Luke", "SUCCESS", "SETcase");
    test_case(connfd, "GET Name", "Luke", "GETcase");
    test_case(connfd, "MOD Name Bssn", "SUCCESS", "MODcase");
    test_case(connfd, "GET Name", "Bssn", "GETcase");
    test_case(connfd, "SET Name Luck", "SUCCESS", "SETcase");
    test_case(connfd, "GET Name", "Luck", "GETcase");
    test_case(connfd, "DEL Name", "SUCCESS", "DELcase");
    test_case(connfd, "GET Name", "NO EXIST", "GETcase");
}

void array_testcase_10w(int connfd)
{
    int count = 100000;
    int i = 0;
    while (i++ < count)
    {
        array_testcase(connfd);
    }
}

int connect_tcpserver(const char *ip, unsigned short port)
{
    int connfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in tcpserver_addr;
    memset(&tcpserver_addr, 0, sizeof(struct sockaddr_in));

    tcpserver_addr.sin_family = AF_INET;
    tcpserver_addr.sin_addr.s_addr = inet_addr(ip);
    tcpserver_addr.sin_port = htons(port);

    int ret = connect(connfd, (struct sockaddr *)&tcpserver_addr, sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        perror("connect");
        return -1;
    }

    return connfd;
}

// array: 0x01, rbtree: 0x02, hash: 0x04, skiptable: 0x08
// ./testcase -s 10.211.55.21 -p 9096 -m 1
int main(int argc, char *argv[])
{
    int ret = 0;

    char ip[16] = {0};
    int port = 0;
    int mode = 1;

    int opt;
    while ((opt = getopt(argc, argv, "s:p:m:?")) != -1)
    {
        switch (opt)
        {
        case 's':
            strcpy(ip, optarg);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'm':
            mode = atoi(optarg);
            break;
        default:
            return -1;
        }
    }

    int connfd = connect_tcpserver(ip, port);

    if (mode & 0x1)
    {
        struct timeval tv_begin;
        gettimeofday(&tv_begin, NULL);

        array_testcase_10w(connfd);
        // array_testcase_100w(connfd);

        struct timeval tv_end;
        gettimeofday(&tv_end, NULL);

        int time_used = TIME_SUB_MS(tv_end, tv_begin);
        printf("80W: time_used: %d, qps: %d\n", time_used, 800000 * 1000 / time_used);
    }

    return 0;
}
