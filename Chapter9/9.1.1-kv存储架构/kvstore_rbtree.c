#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kvstore.h"

#define RED 1
#define BLACK 2

#define ENABLE_TYPE_CHAR 1

#if ENABLE_TYPE_CHAR
typedef char *KEY_TYPE;
#else
typedef int KEY_TYPE;
#endif

typedef struct _rbtree_node
{
    unsigned char color;
    struct _rbtree_node *right;
    struct _rbtree_node *left;
    struct _rbtree_node *parent;
    KEY_TYPE key;
    void *value;
} rbtree_node;

typedef struct _rbtree
{
    rbtree_node *root;
    rbtree_node *nil;
    int count;
} rbtree;

rbtree_node *rbtree_mini(rbtree *T, rbtree_node *x)
{
    while (x->left != T->nil)
    {
        x = x->left;
    }
    return x;
}

rbtree_node *rbtree_maxi(rbtree *T, rbtree_node *x)
{
    while (x->right != T->nil)
    {
        x = x->right;
    }
    return x;
}

rbtree_node *rbtree_successor(rbtree *T, rbtree_node *x)
{
    rbtree_node *y = x->parent;
    if (x->right != T->nil)
    {
        return rbtree_mini(T, x->right);
    }
    while ((y != T->nil) && (x == y->right))
    {
        x = y;
        y = y->parent;
    }
    return y;
}

//左旋-插入和删除时调整树
void rbtree_left_rotate(rbtree *T, rbtree_node *x)
{
    rbtree_node *y = x->right;
    x->right = y->left;
    if (y->left != T->nil)
        y->left->parent = x;

    y->parent = x->parent;
    if (x->parent == T->nil)
        T->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;

    y->left = x;
    x->parent = y;
}

//右旋-插入和删除时调整树（和左旋的实现的代码区别，
// x变成y，y变成x，left变成right，right变成left）
void rbtree_right_rotate(rbtree *T, rbtree_node *y)
{
    rbtree_node *x = y->left;
    y->left = x->right;
    if (x->right != T->nil)
    {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == T->nil)
    {
        T->root = x;
    }
    else if (y == y->parent->right)
    {
        y->parent->right = x;
    }
    else
    {
        y->parent->left = x;
    }
    x->right = y;
    y->parent = x;
}

void rbtree_insert_fixup(rbtree *T, rbtree_node *z)
{
    while (z->parent->color == RED)
    {
        if (z->parent == z->parent->parent->left)
        {
            rbtree_node *y = z->parent->parent->right;
            if (y->color == RED)
            {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;

                z = z->parent->parent;
            }
            else
            {
                if (z == z->parent->right)
                {
                    z = z->parent;
                    rbtree_left_rotate(T, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rbtree_right_rotate(T, z->parent->parent);
            }
        }
        else
        {
            rbtree_node *y = z->parent->parent->left;
            if (y->color == RED)
            {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            }
            else
            {
                if (z == z->parent->left)
                {
                    z = z->parent;
                    rbtree_right_rotate(T, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rbtree_left_rotate(T, z->parent->parent);
            }
        }
    }
    T->root->color = BLACK;
}

void rbtree_insert(rbtree *T, rbtree_node *z)
{
#if ENABLE_TYPE_CHAR

    rbtree_node *y = T->nil;
    rbtree_node *x = T->root;
    while (x != T->nil)
    {
        y = x;
        if (strcmp(z->key, x->key) < 0)
            x = x->left;
        else if (strcmp(z->key, x->key) > 0)
            x = x->right;
        else
            return;
    }

    z->parent = y;
    if (y == T->nil)
        T->root = z;
    else if (strcmp(z->key, y->key) < 0)
        y->left = z;
    else
        y->right = z;

    z->left = T->nil;
    z->right = T->nil;
    z->color = RED;
    rbtree_insert_fixup(T, z);

#else

    rbtree_node *y = T->nil;
    rbtree_node *x = T->root;
    while (x != T->nil)
    {
        y = x;
        if (z->key < x->key)
            x = x->left; //插入的节点值在树的左边
        else if (z->key > x->key)
            x = x->right;
        else
        {
            return;
        }
    }

    z->parent = y;
    if (y == T->nil)
        T->root = z;
    else if (z->key < y->key)
        y->left = z;
    else
        y->right = z;

    z->left = T->nil;
    z->right = T->nil;
    z->color = RED;
    rbtree_insert_fixup(T, z);

#endif
}

void rbtree_delete_fixup(rbtree *T, rbtree_node *x)
{
    while ((x != T->root) && (x->color == BLACK))
    {
        if (x == x->parent->left)
        {
            rbtree_node *w = x->parent->right;
            if (w->color == RED)
            {
                w->color = BLACK;
                x->parent->color = RED;
                rbtree_left_rotate(T, x->parent);
                w = x->parent->right;
            }
            if ((w->left->color == BLACK) && (w->right->color == BLACK))
            {
                w->color = RED;
                x = x->parent;
            }
            else
            {
                if (w->right->color == BLACK)
                {
                    w->left->color = BLACK;
                    w->color = RED;
                    rbtree_right_rotate(T, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rbtree_left_rotate(T, x->parent);
                x = T->root;
            }
        }
        else
        {
            rbtree_node *w = x->parent->left;
            if (w->color == RED)
            {
                w->color = BLACK;
                x->parent->color = RED;
                rbtree_right_rotate(T, x->parent);
                w = x->parent->left;
            }
            if ((w->left->color == BLACK) && (w->right->color == BLACK))
            {
                w->color = RED;
                x = x->parent;
            }
            else
            {
                if (w->left->color == BLACK)
                {
                    w->right->color = BLACK;
                    w->color = RED;
                    rbtree_left_rotate(T, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rbtree_right_rotate(T, x->parent);
                x = T->root;
            }
        }
    }
    x->color = BLACK;
}

rbtree_node *rbtree_delete(rbtree *T, rbtree_node *z)
{
    rbtree_node *y = T->nil;
    rbtree_node *x = T->nil;
    if ((z->left == T->nil) || (z->right == T->nil))
    {
        y = z;
    }
    else
    {
        y = rbtree_successor(T, z);
    }
    if (y->left != T->nil)
    {
        x = y->left;
    }
    else if (y->right != T->nil)
    {
        x = y->right;
    }
    x->parent = y->parent;
    if (y->parent == T->nil)
    {
        T->root = x;
    }
    else if (y == y->parent->left)
    {
        y->parent->left = x;
    }
    else
    {
        y->parent->right = x;
    }
    if (y != z)
    {
#if ENABLE_TYPE_CHAR
        void *tmp = z->key;

        z->key = y->key;
        y->key = tmp;

        tmp = z->value;
        z->value = y->value;
        y->value = tmp;
#else
        z->key = y->key;
        z->value = y->value;
#endif
    }
    if (y->color == BLACK)
    {
        rbtree_delete_fixup(T, x);
    }
    return y;
}

rbtree_node *rbtree_search(rbtree *T, KEY_TYPE key)
{
    rbtree_node *node = T->root;
    while (node != T->nil)
    {
#if ENABLE_TYPE_CHAR
        if (strcmp(key, node->key) < 0)
        {
            node = node->left;
        }
        else if (strcmp(key, node->key) > 0)
        {
            node = node->right;
        }
        else
        {
            return node;
        }
#else
        if (key < node->key)
        {
            node = node->left;
        }
        else if (key > node->key)
        {
            node = node->right;
        }
        else
        {
            return node;
        }
#endif
    }
    return T->nil;
}

void rbtree_traversal(rbtree *T, rbtree_node *node)
{
    if (node != T->nil)
    {
        rbtree_traversal(T, node->left);
#if ENABLE_TYPE_CHAR
        printf("%s : %s\n", node->key, (char *)node->value);
#else
        printf("key:%d, color:%d\n", node->key, node->color);
#endif
        rbtree_traversal(T, node->right);
    }
}

/* 为 KVstore 适配接口 */
rbtree_t Tree;
// 创建一棵红黑树，成功返回 tree， 失败返回 -1
int rbtree_create(rbtree_t *tree)
{
    if (!tree)
        return -1;
    memset(tree, 0, sizeof(rbtree_t));

    tree->nil = kvstore_malloc(1);
    tree->nil->key = kvstore_malloc(1);
    *(tree->nil->key) = '\0';
    tree->nil->color = BLACK;

    tree->root = tree->nil;

    tree->count = 0;

    return 0;
}

// 析构函数：销毁一棵红黑树
void rbtree_destroy(rbtree_t *tree)
{
    if (!tree)
        return;
    if (tree->nil->key)
        kvstore_free(tree->nil->key);

    rbtree_node *cur = tree->root;
    while (cur != tree->nil)
    {
        cur = rbtree_mini(tree, tree->root);
        if (cur == tree->nil)
            break;

        cur = rbtree_delete(tree, cur);

        if (!cur)
        {
            kvstore_free(cur->key);
            kvstore_free(cur->value);
            kvstore_free(cur);
        }
    }
}

/**
 * @description: 判断 key 在红黑树中是否存在。
 * @param {rbtree_t} *tree
 * @param {char} *key
 * @return {*} 如果存在则返回 0；否则返回 -1
 */
int rbtree_exist(rbtree_t *tree, char *key)
{
    // 搜索确定 key 是否存在
    rbtree_node *node_search = rbtree_search(tree, key);

    if (node_search != tree->nil)
        return 0;

    return -1;
}

// 以红黑树为数据结构的 SET 方法；操作成功返回 0，否则返回 -1
int rbtree_set(rbtree_t *tree, char *key, char *value)
{
    rbtree_node *node_search = rbtree_search(tree, key);

    if (node_search != tree->nil) // 如果 key 已经存在，则更新其值
    {
        int ret = rbtree_mod(tree, key, value);
        if (ret == 0)
        {
            LOG("kvstore_rbtree_set: key exist already, update key to new value\n");
            return 0;
        }
        else
            return -1;
    }
    else // 如果 key 不存在，则创建并插入新节点
    {
        rbtree_node *node = (rbtree_node *)kvstore_malloc(sizeof(rbtree_node)); //分配节点内存
        if (!node)
            return -1;

        node->key = kvstore_malloc(strlen(key) + 1);
        if (node->key == NULL)
            return -1;
        memset(node->key, 0, strlen(key) + 1);
        strcpy(node->key, key);

        node->value = malloc(strlen(value) + 1);
        if (node->value == NULL)
        {
            kvstore_free(node->key);
            kvstore_free(node);
            return -1;
        }
        memset(node->value, 0, strlen(value) + 1);
        strcpy((char *)node->value, value);

        rbtree_insert(tree, node);
        tree->count++;

        return 0;
    }
}

// 以红黑树为数据结构的 GET 方法；操作成功返回 value，否则返回 NULL
char *rbtree_get(rbtree_t *tree, char *key)
{
    rbtree_node *node = rbtree_search(tree, key);
    if (node == tree->nil)
        return NULL;

    return node->value;
}

// 以红黑树为数据结构的 DEL 方法；操作成功返回 0，否则返回 -1
int rbtree_del(rbtree_t *tree, char *key)
{
    rbtree_node *node = rbtree_search(tree, key);
    if (node == tree->nil)
    {
        return -1;
    }

    rbtree_node *cur = rbtree_delete(tree, node);
    if (!cur)
    {
        free(cur->key);
        free(cur->value);
        free(cur);
    }
    tree->count--;

    return 0;
}

// 以红黑树为数据结构的 MOD 方法；操作成功返回 0，否则返回 -1
int rbtree_mod(rbtree_t *tree, char *key, char *newValue)
{
    rbtree_node *node = rbtree_search(tree, key);
    if (node == tree->nil)
    {
        return -1;
    }

    node->value = kvstore_malloc(strlen(newValue) + 1);
    if (node->value == NULL)
    {
        return -1;
    }
    strcpy(node->value, newValue);

    return 0;
}

// 以红黑树为数据结构的 COUNT 方法；操作成功返回 红黑树的节点数量
int rbtree_count(rbtree_t *tree)
{
    return tree->count;
}
