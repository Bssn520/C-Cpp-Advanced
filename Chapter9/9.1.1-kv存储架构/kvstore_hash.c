#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kvstore.h"

#define ENABLE_POINTER_KEY 1

#define MAX_TABLE_SIZE 1024
#define MAX_KEY_LEN 128
#define MAX_VALUE_LEN 512

typedef struct hashnode_s
{
#if ENABLE_POINTER_KEY
    char *key;
    char *value;
#else
    char key[MAX_KEY_LEN];
    char value[MAX_VALUE_LEN];
#endif
    struct hashnode_s *next;
} hashnode_t;
typedef struct hashtable_s
{
    hashnode_t **nodes;
    int max_slots;
    int count;
} hashtable_t;

hashtable_t Hash;

static int _hash(char *key, int size)
{

    if (!key)
        return -1;

    int sum = 0;
    int i = 0;

    while (key[i] != 0)
    {
        sum += key[i];
        i++;
    }

    return sum % size;
}

hashnode_t *_create_node(char *key, char *value)
{

    hashnode_t *node = (hashnode_t *)kvstore_malloc(sizeof(hashnode_t));
    if (!node)
        return NULL;

#if ENABLE_POINTER_KEY

    node->key = kvstore_malloc(strlen(key) + 1);
    if (!node->key)
    {
        kvstore_free(node);
        return NULL;
    }
    strcpy(node->key, key);

    node->value = kvstore_malloc(strlen(value) + 1);
    if (!node->value)
    {
        kvstore_free(node->key);
        kvstore_free(node);
        return NULL;
    }
    strcpy(node->value, value);

#else

    strncpy(node->key, key, MAX_KEY_LEN);
    strncpy(node->value, value, MAX_VALUE_LEN);

#endif

    node->next = NULL;

    return node;
}


int create_hashtable(hashtable_t *hash)
{
    if (!hash)
        return -1;
    hash->nodes = (hashnode_t **)kvstore_malloc(sizeof(hashnode_t *) * MAX_TABLE_SIZE);
    if (!hash->nodes)
        return -1;

    hash->max_slots = MAX_TABLE_SIZE;
    hash->count = 0;

    return 0;
}

void destroy_hashtable(hashtable_t *hash)
{
    if (!hash)
        return;

    int i = 0;
    for (i = 0; i < hash->max_slots; i++)
    {
        hashnode_t *node = hash->nodes[i];

        while (node != NULL)
        {
            hashnode_t *tmp = node;
            node = node->next;
            hash->nodes[i] = node;

            kvstore_free(tmp);
        }
    }

    kvstore_free(hash->nodes);
}

/**
 * @description: 判断 key 在 HashTable 中是否存在。
 * @param {hashtable_t} *hash
 * @param {char} *key
 * @return {*} 如果存在则返回 0；否则返回 -1
 */
int kv_exist(hashtable_t *hash, char *key)
{
    char *value = get_kv_hashtable(hash, key);
    if (value)
        return 0;

    return -1;
}

static int _mod_kv_hashtable(hashtable_t *hash, char *key, char *value)
{
    if (!hash || !key || !value)
        return -1;

    int idx = _hash(key, MAX_TABLE_SIZE);

    hashnode_t *node = hash->nodes[idx];

    while (node != NULL)
    {
        if (strcmp(node->key, key) == 0)
        {
            kvstore_free(node->value);

            node->value = kvstore_malloc(strlen(value) + 1);
            if (node->value)
            {
                strcpy(node->value, value);
                return 0;
            }
            else
                assert(0);
        }
        node = node->next;
    }
    return -1;
}

int put_kv_hashtable(hashtable_t *hash, char *key, char *value)
{
    if (!hash || !key || !value)
        return -1;
#if 0
    int idx = _hash(key, MAX_TABLE_SIZE);

    hashnode_t *node = hash->nodes[idx];

    while (node != NULL)
    {
        if (strcmp(node->key, key) == 0) // 如果 key 已经存在，则更新 key对应的 value
        {
            int ret = mod_kv_hashtable(hash, key, value);
            if (ret == 0)
                return 0;
            return -1;
        }
        node = node->next;
    }

    // 如果 key 不存在，则创建新节点并添加到 hashtable
    hashnode_t *new_node = _create_node(key, value);
    new_node->next = hash->nodes[idx];
    hash->nodes[idx] = new_node;

    hash->count++;

    return 0;
#endif

    int idx = _hash(key, MAX_TABLE_SIZE);

    // 如果 key 已经存在
    if (kv_exist(hash, key) != -1)
    {
        int ret = _mod_kv_hashtable(hash, key, value);
        if (ret == 0)
            return 0;
        return -1;
    }

    // 如果 key 不存在，则创建新节点并添加到 hashtable
    hashnode_t *new_node = _create_node(key, value);
    new_node->next = hash->nodes[idx];
    hash->nodes[idx] = new_node;

    hash->count++;

    return 0;
}


char *get_kv_hashtable(hashtable_t *hash, char *key)
{
    if (!hash || !key)
        return NULL;

    int idx = _hash(key, MAX_TABLE_SIZE);

    hashnode_t *node = hash->nodes[idx];

    while (node != NULL)
    {

        if (strcmp(node->key, key) == 0)
        {
            return node->value;
        }

        node = node->next;
    }


    return NULL;
}


int count_kv_hashtable(hashtable_t *hash)
{
    return hash->count;
}

int delete_kv_hashtable(hashtable_t *hash, char *key)
{
    if (!hash || !key)
        return -2;

    int idx = _hash(key, MAX_TABLE_SIZE);

    hashnode_t *head = hash->nodes[idx];
    if (head == NULL)
        return -1; // noexist
    // head node
    if (strcmp(head->key, key) == 0)
    {
        hashnode_t *tmp = head->next;
        hash->nodes[idx] = tmp;

#if ENABLE_POINTER_KEY
        if (head->key)
        {
            kvstore_free(head->key);
        }
        if (head->value)
        {
            kvstore_free(head->value);
        }
        kvstore_free(head);
#else
        free(head);
#endif
        hash->count--;

        return 0;
    }

    hashnode_t *cur = head;
    while (cur->next != NULL)
    {
        if (strcmp(cur->next->key, key) == 0)
            break; // search node

        cur = cur->next;
    }

    if (cur->next == NULL)
    {

        return -1;
    }

    hashnode_t *tmp = cur->next;
    cur->next = tmp->next;
#if ENABLE_POINTER_KEY
    if (tmp->key)
    {
        kvstore_free(tmp->key);
    }
    if (tmp->value)
    {
        kvstore_free(tmp->value);
    }
    kvstore_free(tmp);
#else
    free(tmp);
#endif
    hash->count--;

    return 0;
}

int count_hashtable(hashtable_t *hash)
{
    return hash->count;
}
