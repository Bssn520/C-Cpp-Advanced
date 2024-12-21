#include "kvstore.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define KVSTORE_MAX_TOKENS 128

// 定义协议中使用的命令
const char *commands[] = {
    "SET", "GET", "DEL", "MOD", "COUNT",
    "RSET", "RGET", "RDEL", "RMOD", "RCOUNT"};
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

#if ENABLE_RBTREE_KVENGINE

int kvstore_rbtree_set(char *key, char *value)
{
    return rbtree_set(&tree, key, value);
}

char *kvstore_rbtree_get(char *key)
{
    return rbtree_get(&tree, key);
}

int kvstore_rbtree_del(char *key)
{
    return rbtree_del(&tree, key);
}

int kvstore_rbtree_mod(char *key, char *value)
{
    return rbtree_mod(&tree, key, value);
}

int kvstore_rbtree_count(void)
{
    return rbtree_count(&tree);
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
            LOG("--- DEL %d ---\n\n", ret);
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
            LOG("--- MOD %d ---\n\n", ret);
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

int init_kvengine(void)
{
#if ENABLE_ARRAY_KVENGINE
#endif

#if ENABLE_RBTREE_KVENGINE
    int ret = rbtree_create(&tree);
    if (ret == 0)
        return 0;
#endif
    LOG("Error: 未启用任何存储引擎\n");
    return -1;
}

int main(int argc, const char **argv)
{
    // 存储组件
    init_kvengine();

// 网络组件
#if (ENABLE_NETWORK_SELECT == NETWORK_EPOLL)
    epoll_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_NTYCO)
    ntyco_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_IOURING)
#endif

    return 0;
}
