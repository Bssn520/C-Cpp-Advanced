#include "kvstore.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define KVSTORE_MAX_TOKENS 128

// 定义协议中使用的命令
const char *commands[] = {"SET", "GET", "DEL", "MOD"};
enum
{
    KVS_CMD_START = 0,
    KVS_CMD_SET = KVS_CMD_START,
    KVS_CMD_GET = 1,
    KVS_CMD_DEL = 2,
    KVS_CMD_MOD = 3,
    KVS_CMD_COUNT = 4
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
    for (cmd = KVS_CMD_START; cmd < KVS_CMD_COUNT; cmd++)
    {
        if (strcmp(commands[cmd], tokens[0]) == 0)
            break;
    }

    char *msg = item->wbuffer;
    memset(msg, 0, BUFFER_LENGTH);
    switch (cmd)
    {
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
    default:
    {
        LOG("cmd: %s\n", commands[cmd]);
        assert(0);
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

int main(int argc, const char **argv)
{
#if (ENABLE_NETWORK_SELECT == NETWORK_EPOLL)
    epoll_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_NTYCO)
    ntyco_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_IOURING)
#endif

    return 0;
}
