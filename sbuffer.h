/**
 * \author Nicolas Gutierrez Suarez
 */

#ifndef _SBUFFER_H_
#define _SBUFFER_H_

#include "config.h"

#define SBUFFER_FAILURE -1
#define SBUFFER_SUCCESS 0
#define SBUFFER_NO_DATA 1

/**
 * Basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t *data;         /**< a structure containing the data */
} sbuffer_node_t;

/**
 * A structure to keep track of the buffer.
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last_log node in the buffer */
};

typedef struct sbuffer sbuffer_t;

/**
 * Allocates and initializes a new shared buffer.
 */
void sbuffer_init();

/**
 * All allocated resources are freed and cleaned up.
 */
void sbuffer_free();

/**
 * Reads the first sensor data in 'buffer' (at the 'head') and returns this sensor data as '*data'
 * If 'buffer' is empty, the function doesn't block until new sensor data becomes available but returns SBUFFER_NO_DATA
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to pre-allocated sensor_data_t space, the data will be copied into this structure. No new memory is allocated for 'data' in this function.
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occurred
 */
int sbuffer_read(sbuffer_node_t **node, sensor_data_t **data);

/**
 * Inserts the sensor data in 'data' at the end of 'buffer' (at the 'tail')
 * \param buffer a pointer to the buffer that is used
 * \param data a pointer to sensor_data_t data, that will be copied into the buffer
 * \return SBUFFER_SUCCESS on success and SBUFFER_FAILURE if an error occured
*/
int sbuffer_insert(sensor_data_t *data);

#endif  //_SBUFFER_H_
