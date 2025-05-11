#include <stddef.h>

typedef struct llist_node llist_node;

struct llist_node{
    void* data;
    struct llist_node* next;
};

llist_node* llist_node_create(void* data);

void llist_node_destroy(llist_node*);


typedef struct {
    struct llist_node* head;
    struct llist_node* last;
    size_t len;
} llist;

void llist_new(llist*);

void llist_del(llist*);

void llist_append(llist*, void* data);

void llist_clear(llist*);
