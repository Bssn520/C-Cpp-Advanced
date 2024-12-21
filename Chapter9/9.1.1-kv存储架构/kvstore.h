#ifndef _KV_STORE_H_

#include <stddef.h> // size_t
#include <stdio.h> // fprintf

/* KV存储使用的组件选择 */
// 日志打印宏定义
#define ENABLE_LOG
#ifdef ENABLE_LOG
#define LOG(_fmt, ...) fprintf(stdout, "[%s : %d]: " _fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG(_fmt, ...)
#endif // ENABLE_LOG

// 是否开启KV存储相关的存储引擎
#define ENABLE_ARRAY_KVENGINE 1
#define ENABLE_RBTREE_KVENGINE 1

// 选择 KV存储 使用的网络组件
#define NETWORK_EPOLL 0
#define NETWORK_NTYCO 1
#define NETWORK_IOURING 2
#define ENABLE_NETWORK_SELECT NETWORK_NTYCO


/* 基础网络组件 */
// 网络组件入口函数接口
int epoll_entry(void);
int ntyco_entry(void);

// 每个连接的连接类
#define BUFFER_LENGTH 1024
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


/* KV 存储对应的一些接口 */
void *kvstore_malloc(size_t size);
void kvstore_free(void *ptr);
int kvstore_request(connection_t *item);


/* 基础存储组件 */
// 数组引擎对应的CURD接口
int kvstore_array_set(char *key, char *value);
char *kvstore_array_get(char *key);
int kvstore_array_del(char *key);
int kvstore_array_mod(char *key, char *value);
int kvstore_array_count(void);
// 使用 数组 作为KV存储的数据结构
#if ENABLE_ARRAY_KVENGINE
#define KVS_ARRAY_SIZE 1024
struct kvs_array_item
{
    char *key;
    char *value;
};
#endif

// 红黑树引擎对应的CURD接口
#if ENABLE_RBTREE_KVENGINE

typedef struct _rbtree rbtree_t;
extern rbtree_t tree;

// 初始化红黑树
int rbtree_create(rbtree_t *tree);
// 以红黑树为数据结构的 SET 方法；操作成功返回 0，否则返回 -1
int rbtree_set(rbtree_t *tree, char *key, char *value);
// 以红黑树为数据结构的 GET 方法；操作成功返回 value，否则返回 NULL
char *rbtree_get(rbtree_t *tree, char *key);
// 以红黑树为数据结构的 DEL 方法；操作成功返回 0，否则返回 -1
int rbtree_del(rbtree_t *tree, char *key);
// 以红黑树为数据结构的 MOD 方法；操作成功返回 0，否则返回 -1
int rbtree_mod(rbtree_t *tree, char *key, char *newValue);
// 以红黑树为数据结构的 COUNT 方法；操作成功返回 红黑树的节点数量
int rbtree_count(rbtree_t *tree);

#endif

#endif // !_KV_STORE_H_
