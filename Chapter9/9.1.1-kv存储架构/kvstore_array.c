#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kvstore.h"

/*
返回值    结果
0       操作成功
>0      NOT EXIST
<0      ERROR

NULL    ERROR | NOT EXIST
*/

struct kvs_array_item array_table[KVS_ARRAY_SIZE];

int array_index = 0;

// 判断 key 是否已经存在。返回值： 如果存在，返回的是 key 的索引；如果不存在， 返回-1 。
int kvstore_key_exist(char *key)
{
    for (int i = 0; i < array_index; i++)
    {
        if (strcmp(array_table[i].key, key) == 0)
            return i;
    }

    return -1;
}

// SET 方法对应的操作
int kvstore_array_set(char *key, char *value)
{
    if (key == NULL || value == NULL || array_index == KVS_ARRAY_SIZE)
        return -1;

    // 如果 key 已经存在
    if (kvstore_key_exist(key) != -1)
    {
        LOG("kvstore_array_set: key exist already, update key to new value\n\n");
        kvstore_array_mod(key, value);
        return 0;
    }

    // 如果 key 不存在
    char *kcopy = kvstore_malloc(strlen(key) + 1);
    if (kcopy == NULL)
        return -1;
    strncpy(kcopy, key, strlen(key) + 1);

    char *vcopy = kvstore_malloc(strlen(value) + 1);
    if (vcopy == NULL)
    {
        kvstore_free(kcopy);
        kcopy = NULL;
        return -1;
    }
    strncpy(vcopy, value, strlen(value) + 1);

    array_table[array_index].key = kcopy;
    array_table[array_index].value = vcopy;

    array_index++;

    return 0;
}

// GET 方法对应的操作
char *kvstore_array_get(char *key)
{
    if (key == NULL)
        return NULL;

    int i = kvstore_key_exist(key);
    if (i == -1) // 如果 key 不存在
        return NULL;

    // 如果 key 存在，则返回对应的 value
    return array_table[i].value;
}

// DEL 方法对应的操作
int kvstore_array_del(char *key)
{
    if (key == NULL)
        return -1;

    int i = kvstore_key_exist(key);
    if (i == -1) // 如果 key 不存在
        return 1;

    // 如果 key 存在
    kvstore_free(array_table[i].key);
    kvstore_free(array_table[i].value);
    array_table[i].key = NULL;
    array_table[i].value = NULL;

    array_index--;

    return 0;
}

// MOD 方法对应的操作
int kvstore_array_mod(char *key, char *newValue)
{
    if (key == NULL)
        return -1;

    int i = kvstore_key_exist(key);

    if (i == -1) // 如果 key 不存在
        return 1;

    // 如果 key 存在
    char *tmp = array_table[i].value;

    char *vcopy = kvstore_malloc(strlen(newValue) + 1);
    if (vcopy == NULL)
    {
        printf("kvstore_malloc: newValue\n");
        return -1;
    }

    strncpy(vcopy, newValue, strlen(newValue) + 1);
    array_table[i].value = vcopy;

    kvstore_free(tmp);
    tmp = NULL;

    return 0;
}

int kvstore_array_count(void)
{
    return array_index;
}
