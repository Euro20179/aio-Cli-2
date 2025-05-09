#include "llist.h"
#include <stdlib.h>

llist_node* llist_node_create(void* data) {
    llist_node* n = malloc(sizeof(llist_node));
    n->data = data;
    n->next = NULL;
    return n;
}

void llist_node_destroy(llist_node * node) {
    free(node);
}

void llist_new(llist * l) {
    l->head = NULL;
    l->last = NULL;
    l->len = 0;
}

void llist_append(llist* l, void *data) {
    l->last->next = llist_node_create(data);
    l->len++;
    l->last = l->last->next;

    if(l->head == NULL) {
        l->head = l->last;
    }
}
