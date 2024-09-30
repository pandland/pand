#pragma once
#include <stdbool.h>
#include <stddef.h>

struct heap_node {
    struct heap_node *left;
    struct heap_node *right;
    struct heap_node *parent;
};

typedef int (*heap_comparator_t)(struct heap_node *a, struct heap_node *b);

struct heap {
    struct heap_node *root;
    size_t size;
    heap_comparator_t comparator;
};

static void heap_swap(struct heap*, struct heap_node*, struct heap_node*);


static void heap_init(struct heap *h, heap_comparator_t comparator) {
    h->root = NULL;
    h->size = 0;
    h->comparator = comparator;;
}


static void heap_init_node(struct heap_node *node) {
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
}


/* returns min/max node without removing it */
static struct heap_node *heap_peek(struct heap *h) {
    return h->root;
}


static void heap_insert(struct heap *h, struct heap_node *newnode) {
    if (h->root == NULL) {
        h->root = newnode;
        h->size++;

        newnode->parent = NULL;
        newnode->left = NULL;
        newnode->right = NULL;

        return;
    }

    // find last node
    struct heap_node *last = h->root;

    unsigned n = h->size + 1;
    unsigned mask = 0;
    unsigned bits_count = 0;

    while (n >= 2) {
        mask = (mask << 1) | (n & 1);
        bits_count++;
        n >>= 1;
    }

    while (bits_count > 1) {
        if (mask & 1) {
            last = last->right;
        } else {
            last = last->left;
        }

        bits_count--;
        mask >>= 1;
    }

    // node insertion
    if (last->left == NULL) {
        last->left = newnode;
    } else {
        last->right = newnode;
    }

    h->size++;
    newnode->parent = last;
    newnode->left = NULL;
    newnode->right = NULL;

    struct heap_node *node = newnode;

    while (node->parent != NULL && h->comparator(node, node->parent) > 0) {
        heap_swap(h, node, node->parent);
    }
}


static struct heap_node *heap_remove(struct heap *h, struct heap_node *rnode) {
    if (h->root == NULL) {
        return NULL;
    }

    if (h->size == 1) {
        h->root = NULL;
        h->size = 0;
        return rnode;
    }

    struct heap_node *last = h->root;

    unsigned n = h->size;
    unsigned mask = 0;
    unsigned bits_count = 0;

    while (n >= 2) {
        mask = (mask << 1) | (n & 1);
        bits_count++;
        n >>= 1;
    }

    while (bits_count > 0) {
        if (mask & 1) {
            last = last->right;
        } else {
            last = last->left;
        }

        bits_count--;
        mask >>= 1;
    }

    h->size--;

    // unlink last item
    if (last->parent) {
        if (last->parent->left == last)
            last->parent->left = NULL;
        else
            last->parent->right = NULL;
    }

    if (last == rnode)
        return rnode;

    if (rnode->parent) {
        if (rnode->parent->left == rnode)
            rnode->parent->left = last;
        else
            rnode->parent->right = last;
    } else {
        h->root = last;
    }

    if (rnode->left)
        rnode->left->parent = last;

    if (rnode->right)
        rnode->right->parent = last;

    last->left = rnode->left;
    last->right = rnode->right;
    last->parent = rnode->parent;

    while (true) {
        struct heap_node *swap_node = NULL;

        if (last->left && last->right) {
            if (h->comparator(last->left, last->right)) {
                swap_node = last->left;
            } else {
                swap_node = last->right;
            }
        } else if (last->left) {
            swap_node = last->left;
        } else if (last->right) {
            swap_node = last->right;
        }

        if (!swap_node || h->comparator(last, swap_node)) {
            break;
        }

        heap_swap(h, swap_node, last);
    }

    while (last->parent != NULL && h->comparator(last, last->parent)) {
        heap_swap(h, last, last->parent);
    }

    return rnode;
}


/* returns min/max node and removes it from the heap */
static struct heap_node *heap_pop(struct heap *h) {
    return heap_remove(h, h->root);
}


static void heap_swap(struct heap *h, struct heap_node *child, struct heap_node *parent) {
    struct heap_node tmp = *parent;
    struct heap_node *grandparent = parent->parent;

    if (grandparent) {
        if (grandparent->left == parent)
            grandparent->left = child;
        else
            grandparent->right = child;
    }

    if (parent->left)
        parent->left->parent = child;

    if (parent->right)
        parent->right->parent = child;

    parent->left = child->left;
    if (child->left)
        child->left->parent = parent;

    parent->right = child->right;
    if (child->right)
        child->right->parent = parent;

    parent->parent = child;

    // prevent self-reference:
    if (tmp.left == child)
        child->left = parent;
    else
        child->left = tmp.left;

    // prevent self-reference:
    if (tmp.right == child)
        child->right = parent;
    else
        child->right = tmp.right;

    child->parent = tmp.parent;

    // don't forget to set new root...
    if (!child->parent)
        h->root = child;
}
