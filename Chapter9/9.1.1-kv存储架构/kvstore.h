#ifndef _KV_STORE_H_

#include <stddef.h> // size_t
#include <stdio.h> // fprintf

// 日志开关宏
#define ENABLE_LOG

#ifdef ENABLE_LOG
#define LOG(_fmt, ...) fprintf(stdout, "[%s : %d]: " _fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG(_fmt, ...)
#endif // ENABLE_LOG


#define BUFFER_LENGTH 1024

// 每个连接的连接信息类
typedef int (*RCALLBACK)(int fd);
typedef struct connection_item
{
    int fd;
    char rbuffer[BUFFER_LENGTH];
    int rlength;
    char wbuffer[BUFFER_LENGTH];
    int wlength;
    union
    {
        RCALLBACK accept_callback;
        RCALLBACK recv_callback;
    } recv_t;
    RCALLBACK send_callback;
} connection_t;


int epoll_entry(void);
int ntyco_entry(void);

void *kvstore_malloc(size_t size);

void kvstore_free(void *ptr);

int kvstore_request(connection_t *item);

int kvstore_array_set(char *key, char *value);

char *kvstore_array_get(char *key);

int kvstore_array_del(char *key);

int kvstore_array_mod(char *key, char *value);

#define NETWORK_EPOLL 0
#define NETWORK_NTYCO 1
#define NETWORK_IOURING 2
#define ENABLE_NETWORK_SELECT NETWORK_NTYCO

#define KVS_ARRAY_ENABLE 1

#if KVS_ARRAY_ENABLE

struct kvs_array_item
{
    char *key;
    char *value;
};

#define KVS_ARRAY_SIZE 1024

#endif


#endif // !_KV_STORE_H_
