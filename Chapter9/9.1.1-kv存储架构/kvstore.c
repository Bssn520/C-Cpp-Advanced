#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "kvstore.h"

#define KVSTORE_MAX_TOKENS 128

// 定义协议中使用的命令
const char *commands[] = {
    "SET", "GET", "DEL", "MOD", "COUNT",
    "RSET", "RGET", "RDEL", "RMOD", "RCOUNT",
    "HSET", "HGET", "HDEL", "HMOD", "HCOUNT"};
enum
{
    KVS_CMD_START = 0,
    KVS_CMD_SET = KVS_CMD_START,
    KVS_CMD_GET,
    KVS_CMD_DEL,
    KVS_CMD_MOD,
    KVS_CMD_COUNT,

    KVS_CMD_RSET,
    KVS_CMD_RGET,
    KVS_CMD_RDEL,
    KVS_CMD_RMOD,
    KVS_CMD_RCOUNT,

    KVS_CMD_HSET,
    KVS_CMD_HGET,
    KVS_CMD_HDEL,
    KVS_CMD_HMOD,
    KVS_CMD_HCOUNT,
    KVS_CMD_END
};

void *kvstore_malloc(size_t size)
{
    return malloc(size);
}

void kvstore_free(void *ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
    }
}

/* ------------------------ RBTREE ------------------------ */
#if ENABLE_RBTREE_KVENGINE
int kvstore_rbtree_create(rbtree_t *tree)
{
    return rbtree_create(&Tree);
}
void kvstore_rbtree_destroy(rbtree_t *tree)
{
    return rbtree_destroy(&Tree);
}
int kvstore_rbtree_set(char *key, char *value)
{
    return rbtree_set(&Tree, key, value);
}
char *kvstore_rbtree_get(char *key)
{
    return rbtree_get(&Tree, key);
}
int kvstore_rbtree_del(char *key)
{
    return rbtree_del(&Tree, key);
}
int kvstore_rbtree_mod(char *key, char *value)
{
    return rbtree_mod(&Tree, key, value);
}
int kvstore_rbtree_count(void)
{
    return rbtree_count(&Tree);
}
#endif
/* ------------------------ ARRAY ------------------------ */
#if ENABLE_ARRAY_KVENGINE
int kvstore_array_create(array_t *arr)
{
    return array_create(arr);
}
void kvstore_array_destroy(array_t *arr)
{
    return array_destroy(arr);
}
int kvstore_array_set(char *key, char *value)
{
    return array_set(&Array, key, value);
}
char *kvstore_array_get(char *key)
{
    return array_get(&Array, key);
}
int kvstore_array_del(char *key)
{
    return array_del(&Array, key);
}
int kvstore_array_mod(char *key, char *newValue)
{
    return array_mod(&Array, key, newValue);
}
int kvstore_array_count(void)
{
    return array_count(&Array);
}
#endif
/* ------------------------ HASH ------------------------ */
#if ENABLE_HASH_KVENGINE
int kvstore_hash_create(hashtable_t *hash)
{
    return create_hashtable(hash);
}
void kvstore_hash_destroy(hashtable_t *hash)
{
    return destroy_hashtable(hash);
}
int kvstore_hash_set(char *key, char *value)
{
    return put_kv_hashtable(&Hash, key, value);
}
char *kvstore_hash_get(char *key)
{
    return get_kv_hashtable(&Hash, key);
}
int kvstore_hash_del(char *key)
{
    return delete_kv_hashtable(&Hash, key);
}
int kvstore_hash_mod(char *key, char *newValue)
{
    return mod_kv_hashtable(&Hash, key, newValue);
}
int kvstore_hash_count(void)
{
    return count_hashtable(&Hash);
}
#endif

// 用于分割提取发来的数据，如 SET KEY VALUE
int kvstore_split_token(char *msg, char **tokens)
{
    if (msg == NULL || tokens == NULL)
        return -1;

    int index = 0;

    char *token = strtok(msg, " ");

    while (token != NULL)
    {
        tokens[index++] = token;
        token = strtok(NULL, " ");
    }

    return index;
}
// KVStore 协议
int kvstore_parser_protocol(connection_t *item, char **tokens, int count)
{
    if (item == NULL || tokens == NULL || count == 0)
        return -1;

    int cmd;
    for (cmd = KVS_CMD_START; cmd < KVS_CMD_END; cmd++)
    {
        if (strcmp(commands[cmd], tokens[0]) == 0)
            break;
    }

    char *msg = item->wbuffer;
    memset(msg, 0, BUFFER_LENGTH);

    switch (cmd)
    {
    /* 数组结构对应的操作 */
    case KVS_CMD_SET:
    {
        int ret = kvstore_array_set(tokens[1], tokens[2]);
        if (ret == 0)
        {
            LOG("--- SET: %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "SUCCESS");
        }
        else if (ret < 0)
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        break;
    }
    case KVS_CMD_GET:
    {
        char *value = kvstore_array_get(tokens[1]);
        if (value)
        {
            LOG("---The value of %s is %s ---\n\n", tokens[1], value);
            snprintf(msg, BUFFER_LENGTH, "%s", value);
        }
        else
        {
            LOG("--- NO EXIST ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "NO EXIST");
        }
        break;
    }
    case KVS_CMD_DEL:
    {
        int ret = kvstore_array_del(tokens[1]);
        if (ret == 0)
        {
            LOG("--- DEL %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "SUCCESS");
        }
        else if (ret > 0)
        {
            LOG("--- NO EXIST ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "NO EXIST");
        }
        else
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        break;
    }
    case KVS_CMD_MOD:
    {
        int ret = kvstore_array_mod(tokens[1], tokens[2]);
        if (ret == 0)
        {
            LOG("--- MOD %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "SUCCESS");
        }
        else if (ret > 0)
        {
            LOG("--- NO EXIST ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "NO EXIST");
        }
        else
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        break;
    }
    case KVS_CMD_COUNT:
    {
        int ret = kvstore_array_count();
        if (ret < 0)
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        else
        {
            LOG("--- COUNT: %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "%d", ret);
        }
        break;
    }

    /* 红黑树结构对应的操作 */
    case KVS_CMD_RSET:
    {
        int ret = kvstore_rbtree_set(tokens[1], tokens[2]);
        if (ret == 0)
        {
            LOG("--- RSET: %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "SUCCESS");
        }
        else if (ret == -1)
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        break;
    }
    case KVS_CMD_RGET:
    {
        char *value = kvstore_rbtree_get(tokens[1]);
        if (value)
        {
            LOG("---The value of %s is %s ---\n\n", tokens[1], value);
            snprintf(msg, BUFFER_LENGTH, "%s", value);
        }
        else
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        break;
    }
    case KVS_CMD_RDEL:
    {
        int ret = kvstore_rbtree_del(tokens[1]);
        if (ret == 0)
        {
            LOG("--- RDEL %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "SUCCESS");
        }
        else if (ret == -1)
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        break;
    }
    case KVS_CMD_RMOD:
    {
        int ret = kvstore_rbtree_mod(tokens[1], tokens[2]);
        if (ret == 0)
        {
            LOG("--- RMOD %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "SUCCESS");
        }
        else if (ret == -1)
        {
            LOG("--- NO EXIST ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "NO EXIST");
        }
        break;
    }
    case KVS_CMD_RCOUNT:
    {
        int ret = kvstore_rbtree_count();
        if (ret < 0)
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        else
        {
            LOG("--- RCOUNT: %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "%d", ret);
        }
        break;
    }

    /* 哈希结构对应的操作 */
    case KVS_CMD_HSET:
    {
        int ret = kvstore_hash_set(tokens[1], tokens[2]);
        if (ret == 0)
        {
            LOG("--- HSET: %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "SUCCESS");
        }
        else if (ret == -1)
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        break;
    }
    case KVS_CMD_HGET:
    {
        char *value = kvstore_hash_get(tokens[1]);
        if (value)
        {
            LOG("---The value of %s is %s ---\n\n", tokens[1], value);
            snprintf(msg, BUFFER_LENGTH, "%s", value);
        }
        else
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        break;
    }
    case KVS_CMD_HDEL:
    {
        int ret = kvstore_hash_del(tokens[1]);
        if (ret == 0)
        {
            LOG("--- HDEL %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "SUCCESS");
        }
        else if (ret == -1)
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        break;
    }
    case KVS_CMD_HMOD:
    {
        int ret = kvstore_hash_mod(tokens[1], tokens[2]);
        if (ret == 0)
        {
            LOG("--- HMOD %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "SUCCESS");
        }
        else if (ret == -1)
        {
            LOG("--- NO EXIST ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "NO EXIST");
        }
        break;
    }
    case KVS_CMD_HCOUNT:
    {
        int ret = kvstore_hash_count();
        if (ret < 0)
        {
            LOG("--- ERROR ---\n\n");
            snprintf(msg, BUFFER_LENGTH, "ERROR");
        }
        else
        {
            LOG("--- HCOUNT: %d ---\n\n", ret);
            snprintf(msg, BUFFER_LENGTH, "%d", ret);
        }
        break;
    }
    default:
    {
        LOG("CMD: %s 无匹配\n\n", commands[cmd]);
        // assert(0);
    }
    }

    return 0;
}

// 获取并处理来自 reactor 的请求
int kvstore_request(connection_t *item)
{
    LOG("--- recv: %s ---\n", item->rbuffer);

    char *msg = item->rbuffer;

    char *tokens[KVSTORE_MAX_TOKENS];

    int count = kvstore_split_token(msg, tokens);

    for (int i = 0; i < count; ++i)
        LOG("tokens[%d]: %s\n", i, tokens[i]);

    kvstore_parser_protocol(item, tokens, count);

    return 0;
}

// 初始化存储引擎
int init_kvengine(void)
{
#if ENABLE_ARRAY_KVENGINE
    int ret_array_engine = kvstore_array_create(&Array);
    if (ret_array_engine != 0)
        return -1;
#endif

#if ENABLE_RBTREE_KVENGINE
    int ret_rbtree_engine = rbtree_create(&Tree);
    if (ret_rbtree_engine != 0)
        return -1;
#endif

#if ENABLE_HASH_KVENGINE
    int ret_hash_engine = kvstore_hash_create(&Hash);
    if (ret_hash_engine != 0)
        return -1;
#endif
    LOG("引擎已开启\\n");
    return 0;
}
// 退出存储引擎
int exit_kvengine(void)
{
#if ENABLE_ARRAY_KVENGINE
    kvstore_array_destroy(&Array);
#endif

#if ENABLE_RBTREE_KVENGINE
    kvstore_rbtree_destroy(&Tree);
#endif

#if ENABLE_HASH_KVENGINE
    kvstore_hash_destroy(&Hash);
#endif
    LOG("引擎已退出\n");
    return 0;
}

int main(int argc, const char **argv)
{
    // 初始化存储引擎
    init_kvengine();

// 网络组件
#if (ENABLE_NETWORK_SELECT == NETWORK_EPOLL)
    epoll_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_NTYCO)
    ntyco_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_IOURING)
#endif

    // 退出存储引擎
    exit_kvengine();

    return 0;
}
