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
// 使用 数组 作为KV存储的数据结构
#if ENABLE_ARRAY_KVENGINE

#define KVS_ARRAY_SIZE 1024
struct kvs_array_item
{
    char *key;
    char *value;
};

typedef struct array_s
{
    struct kvs_array_item *array_table;
    int array_index;
} array_t;
extern array_t Array;

// 数组引擎对应的CURD接口
int array_create(array_t *arr);
void array_destroy(array_t *arr);
int array_set(array_t *arr, char *key, char *value);
char *array_get(array_t *arr, char *key);
int array_del(array_t *arr, char *key);
int array_mod(array_t *arr, char *key, char *value);
int array_count(array_t *arr);

#endif

// 红黑树引擎对应的CURD接口
#if ENABLE_RBTREE_KVENGINE

typedef struct _rbtree rbtree_t;
extern rbtree_t Tree;

// 初始化红黑树
int rbtree_create(rbtree_t *tree);
// 析构红黑树
void rbtree_destroy(rbtree_t *tree);
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
