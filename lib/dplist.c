/**
 * \author Nicolas Gutierrez Suarez
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "dplist.h"

#define DPLIST_NO_ERROR 0
#define DPLIST_MEMORY_ERROR 1 // error due to mem alloc failure
#define DPLIST_INVALID_ERROR 2 //error due to a list operation applied on a NULL list 

#ifdef DEBUG
#define DEBUG_PRINTF(...) 									                                        \
        do {											                                            \
            fprintf(stderr,"\nIn %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	    \
            fprintf(stderr,__VA_ARGS__);								                            \
            fflush(stderr);                                                                         \
                } while(0)
#else
#define DEBUG_PRINTF(...) (void)0
#endif


#define DPLIST_ERR_HANDLER(condition, err_code)                         \
    do {                                                                \
            if ((condition)) DEBUG_PRINTF(#condition " failed\n");      \
            assert(!(condition));                                       \
        } while(0)

struct dplist_node {
    dplist_node_t *prev, *next;
    void *element;
};

struct dplist {
    dplist_node_t *head;
    void (*element_free)(void **element);
};


dplist_t *dpl_create(void (*element_free)(void **element)) {
    dplist_t *list;
    list = malloc(sizeof(struct dplist));
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_MEMORY_ERROR);
    list->head = NULL;
    list->element_free = element_free;
    return list;
}


void dpl_free(dplist_t **list) {
    // Check if the list is null or if the head is not null (and therefore the list is not empty).
    if (*list == NULL) return;
    else if ((*list)->head != NULL) {
        // We get the first node (the head) and we start iterating through all the nodes until
        // the end is reached (NULL pointer). We free the memory for each node including the head
        // and make sure we free the pointer to the list and the actual dplist instance.
        dplist_node_t *current, *next;

        current = (*list)->head;
        next = current->next;

        while (next != NULL) {
            (**list).element_free(&current->element);
            free(current);
            current = next;
            next = current->next;
        }
        free(current);
    }
    free(*list);
    *list = NULL;
}

dplist_t *dpl_insert_at_index(dplist_t *list, void *element, int index) {
    dplist_node_t *ref_at_index, *list_node;
    if (list == NULL) return NULL;

    list_node = malloc(sizeof(dplist_node_t));
    DPLIST_ERR_HANDLER(list_node == NULL, DPLIST_MEMORY_ERROR);

    list_node->element = element;

    // pointer drawing breakpoint
    if (list->head == NULL) { // covers case 1
        list_node->prev = NULL;
        list_node->next = NULL;
        list->head = list_node;
        // pointer drawing breakpoint
    } else if (index <= 0) { // covers case 2
        list_node->prev = NULL;
        list_node->next = list->head;
        list->head->prev = list_node;
        list->head = list_node;
        // pointer drawing breakpoint
    } else {
        ref_at_index = dpl_get_reference_at_index(list, index);
        assert(ref_at_index != NULL);
        // pointer drawing breakpoint
        if (index < dpl_size(list)) { // covers case 4
            list_node->prev = ref_at_index->prev;
            list_node->next = ref_at_index;
            ref_at_index->prev->next = list_node;
            ref_at_index->prev = list_node;
            // pointer drawing breakpoint
        } else { // covers case 3
            assert(ref_at_index->next == NULL);
            list_node->next = NULL;
            list_node->prev = ref_at_index;
            ref_at_index->next = list_node;
            // pointer drawing breakpoint
        }
    }
    return list;
}


int dpl_size(dplist_t *list) {
    // First check if pointer to list is null or list is empty.
    DPLIST_ERR_HANDLER(list == NULL, DPLIST_INVALID_ERROR);
    if (list->head == NULL) {
        return 0;
    }
    // We go through the list until we get the last_log node and increment the counter
    // until we do. Return the count of nodes.
    int i;

    dplist_node_t *current = list->head;

    for (i = 1; current->next != NULL; ++i) {
        current = current->next;
    }
    return i;
}

void *dpl_get_element_at_index(dplist_t *list, int index) {
    // Keep it simple, stupid.
    return dpl_get_reference_at_index(list, index)->element;
}

dplist_node_t *dpl_get_reference_at_index(dplist_t *list, int index) {
    // The original method supplied was too verbose and not cash money.
    if (list == NULL || list->head == NULL) return NULL;
    dplist_node_t *current;

    // If the desired index is outside the bounds of the list we set the index as the last_log
    // element. When we get to the desired index we return the pointer to the element.
    current = list->head;

    for (int i = 0; i < index; ++i) {
        if (current->next == NULL) return current;
        current = current->next;
    }
    return current;
}


