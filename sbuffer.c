/**
 * \author Nicolas Gutierrez Suarez
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#include "sbuffer.h"


pthread_mutex_t write_lock_mtx;

sbuffer_t *sbuffer;

void sbuffer_init() {
    // Initialize the buffer and the mutex.
    pthread_mutex_init(&write_lock_mtx, NULL);
    sbuffer = malloc(sizeof(sbuffer_t));

    ERROR_HANDLER(sbuffer == NULL, "Buffer malloc failed.");
    sbuffer->head = NULL;
    sbuffer->tail = NULL;

    DEBUG_PRINTF("Buffer initialized");
}

void sbuffer_free() {
    // Free everything, destroy the mutex. Make sure no thread is still writing.
    pthread_mutex_lock(&write_lock_mtx);
    ERROR_HANDLER(sbuffer == NULL, "Buffer is NULL.");

    while (sbuffer->head) {
        sbuffer_node_t *temp = sbuffer->head;
        sbuffer->head = sbuffer->head->next;
        free(temp->data);
        free(temp);
    }
    free(sbuffer);
    sbuffer = NULL;
    pthread_mutex_unlock(&write_lock_mtx);
    pthread_mutex_destroy(&write_lock_mtx); // Destroy the mutex for good measure.
    DEBUG_PRINTF("Buffer freed successfully.");
}

int sbuffer_read(sbuffer_node_t **node, sensor_data_t **data) {
    // No mention is ever made of a bounded buffer, I'll just assume an infinite amount of RAM then :P
    // No deleting reads = no mutex = no headaches.
    ERROR_HANDLER(sbuffer == NULL, "Buffer is NULL.");
    if (sbuffer->head == NULL) return SBUFFER_NO_DATA; //List is empty

    if (*node == NULL) {
        // No node selected, start from head.
        *data = sbuffer->head->data;
        *node = sbuffer->head;
        return SBUFFER_SUCCESS;
    } else if ((*node)->next != NULL) {
        // Node selected, start from there.
        *data = (*node)->next->data;
        *node = (*node)->next;
        return SBUFFER_SUCCESS;
    } else {
        // List has no data.
        return SBUFFER_NO_DATA;
    }
}

int sbuffer_insert(sensor_data_t *data) {
    ERROR_HANDLER(sbuffer == NULL, "Buffer is NULL.");
    sbuffer_node_t *temp = malloc(sizeof(sbuffer_node_t));

    ERROR_HANDLER(temp == NULL, "Buffer malloc failed.");

    temp->data = data;
    temp->next = NULL;

    pthread_mutex_lock(&write_lock_mtx); // Make sure only one thread writes concurrently.

    if (!sbuffer->tail) // buffer empty (buffer->head should also be NULL
    {
        sbuffer->head = sbuffer->tail = temp;
    } else // buffer not empty
    {
        sbuffer->tail->next = temp;
        sbuffer->tail = sbuffer->tail->next;
    }
    pthread_mutex_unlock(&write_lock_mtx);

    return SBUFFER_SUCCESS;
}
