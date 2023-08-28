/**
 * \author Nicolas Gutierrez Suarez
 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <pthread.h>

#include "lib/dplist.h"
#include "config.h"
#include "datamgr.h"
#include "sensor_db.h"
#include "sbuffer.h"

#define SENSOR_MAP_NAME "room_sensor.map"

static dplist_t *data_list; // Static global so no other process can access it.

struct element {
    sensor_id_t sensor_id;
    int room_id; // Why is this even here?
    sensor_value_t data_queue[RUN_AVG_LENGTH];
    sensor_ts_t last_modified;
}; // The data queue should only keep the number of elements needed.

/**
 * The callback to delete the element in a node.
 * @param el The element to be freed.
 */
void element_free(void **el) {
    free(*el);
}

/**
 * This calls the free function in dpl and checks its output.
 */
static void datamgr_free() {
    dpl_free(&data_list);
    ERROR_HANDLER(data_list != NULL, "Error freeing list");
}

/**
 * This function gets the average for all the saved values in a list.
 * @param data_queue the list of saved values
 * @return The average of all readings.
 */
static sensor_value_t datamgr_get_avg(sensor_value_t const data_queue[RUN_AVG_LENGTH]) {
    float cum_sum = 0;
    for (int j = 0; j < RUN_AVG_LENGTH; ++j) {
        cum_sum += data_queue[j];
    }
    return cum_sum / (float) RUN_AVG_LENGTH; // If you don't cast it, it truncates the decimals.
}

void *datamgr_init() {
    struct sensor_mapping {
        int room_id;
        int sensor_id;
    } sm; // Simple struct to hold the "key-value" pairs.

    // Open the sensor map, create the list to hold the data.
    FILE *fp_sensor_map = fopen(SENSOR_MAP_NAME, "r");
    ERROR_HANDLER(!fp_sensor_map, "Map file not read correctly.");
    data_list = dpl_create(element_free);

    while (1) {
        // Get all the lines from the map and make a new element in the list for each one.
        fscanf(fp_sensor_map, "%i %i", &sm.room_id, &sm.sensor_id);
        if (feof(fp_sensor_map)) break;

        element_t *el = malloc(sizeof(element_t));

        el->sensor_id = sm.sensor_id;
        el->room_id = sm.room_id;
        el->last_modified = 0; // Placeholder until data comes in.

        for (int i = 0; i < RUN_AVG_LENGTH; ++i) {
            // Set the initial values in queue between the range of temps so the average moves from there.
            el->data_queue[i] = ((float) (DSET_MIN_TEMP + DSET_MAX_TEMP)) / 2.0;
        }

        dpl_insert_at_index(data_list, el, 0);
    }
    DEBUG_PRINTF("Started Data Manager");

    sbuffer_node_t *node = NULL; //This makes the datamgr remember its last position.
    sensor_data_t *data;
    do {
        // Get all the data in the buffer, if no data is available, sleep for 1 us waiting for new data to come in.
        do {
            int res = sbuffer_read(&node, &data);
            if (res != SBUFFER_NO_DATA) break;
            usleep(1); // Requires GNU_SOURCE
        } while (1);

        if (data->id == 0) break; // Stop if EOF in buffer.

        // Find matching sensor id in list and store its index in idx.
        element_t *tmp;
        int found = false;
        for (int i = 0; i < dpl_size(data_list); ++i) {
            tmp = (element_t *) dpl_get_element_at_index(data_list, i);
            if (tmp->sensor_id == data->id) {
                found = true;
                break;
            }
        }

        // If the sensor exists, insert the newest data to the array, calculate the running average, and check
        // if it surpasses the preset limits.
        if (found) {
            DEBUG_PRINTF("Datum read: %i %f %li", data->id, data->value, data->ts);

            // We shift the queue right and insert the newest value at the initial position.
            for (int i = RUN_AVG_LENGTH - 1; i > 0; --i) {
                tmp->data_queue[i] = tmp->data_queue[i - 1];
            }
            tmp->data_queue[0] = data->value;
            tmp->last_modified = data->ts;

            // Check the average of the newly updated queue. Log them if they are outside the set range.
            sensor_value_t avg = datamgr_get_avg(tmp->data_queue);
            if (avg > DSET_MAX_TEMP) {
                DEBUG_PRINTF("Sensor %i too hot %f > %d", data->id, avg, DSET_MAX_TEMP);
                log_pipe_write(LOG_TOO_HOT, data->id, avg);
            } else if (avg < DSET_MIN_TEMP) {
                DEBUG_PRINTF("Sensor %i too cold %f < %d", data->id, avg, DSET_MIN_TEMP);
                log_pipe_write(LOG_TOO_COLD, data->id, avg);
            }
        } else {
            // Log that the sensor id is wrong.
            DEBUG_PRINTF("Sensor %i not in map", data->id);
            log_pipe_write(LOG_INVALID_ID, data->id, 0);
        }
    } while (1);

    datamgr_free();
    pthread_exit(NULL);
}


