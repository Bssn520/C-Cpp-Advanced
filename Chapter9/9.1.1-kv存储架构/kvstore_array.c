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

array_t Array;

int array_create(array_t *arr)
{
    if (arr)
    {
        arr->array_table = kvstore_malloc(sizeof(struct kvs_array_item) * KVS_ARRAY_SIZE);
        if (arr->array_table)
        {
            memset(arr->array_table, 0, sizeof(struct kvs_array_item) * KVS_ARRAY_SIZE);
            arr->array_index = 0;
            return 0;
        }
    }
    return -1;
}

void array_destroy(array_t *arr)
{
    if (!arr)
        return;

    if (!arr->array_table)
        kvstore_free(arr->array_table);
}

// 判断 key 是否已经存在。返回值： 如果存在，返回的是 key 的索引；如果不存在， 返回-1 。
static int key_exist(array_t *arr, char *key)
{
    if (arr == NULL)
        return -1;

    for (int i = 0; i < arr->array_index; i++)
    {
        if (strcmp(arr->array_table[i].key, key) == 0)
            return i;
    }

    return -1;
}

// SET 方法对应的操作
int array_set(array_t *arr, char *key, char *value)
{
    if (arr == NULL || key == NULL || value == NULL || arr->array_index == KVS_ARRAY_SIZE)
        return -1;

    // 如果 key 已经存在
    if (key_exist(arr, key) != -1)
    {
        LOG("kvstore_array_set: key exist already, update key to new value\n\n");
        array_mod(arr, key, value);
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

    arr->array_table[arr->array_index].key = kcopy;
    arr->array_table[arr->array_index].value = vcopy;

    arr->array_index++;

    return 0;
}

// GET 方法对应的操作
char *array_get(array_t *arr, char *key)
{
    if (arr == NULL || key == NULL)
        return NULL;

    int i = key_exist(arr, key);
    if (i == -1) // 如果 key 不存在
        return NULL;

    // 如果 key 存在，则返回对应的 value
    return arr->array_table[i].value;
}

// DEL 方法对应的操作
int array_del(array_t *arr, char *key)
{
    if (key == NULL)
        return -1;

    int i = key_exist(arr, key);
    if (i == -1) // 如果 key 不存在
        return -1;

    // 如果 key 存在
    kvstore_free(arr->array_table[i].key);
    kvstore_free(arr->array_table[i].value);
    arr->array_table[i].key = NULL;
    arr->array_table[i].value = NULL;

    // 将后续的元素向前移动
    for (int j = i; j < arr->array_index - 1; j++)
    {
        arr->array_table[j] = arr->array_table[j + 1];
    }
    arr->array_index--;

    return 0;
}

// MOD 方法对应的操作
int array_mod(array_t *arr, char *key, char *newValue)
{
    if (key == NULL || arr == NULL)
        return -1;

    int i = key_exist(arr, key);

    if (i == -1) // 如果 key 不存在
        return 1;

    // 如果 key 存在
    char *tmp = arr->array_table[i].value;

    char *vcopy = kvstore_malloc(strlen(newValue) + 1);
    if (vcopy == NULL)
    {
        printf("kvstore_malloc: newValue\n");
        return -1;
    }

    strncpy(vcopy, newValue, strlen(newValue) + 1);
    arr->array_table[i].value = vcopy;

    kvstore_free(tmp);
    tmp = NULL;

    return 0;
}

int array_count(array_t *arr)
{
    return arr->array_index;
}
