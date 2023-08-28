/**
 * \author Nicolas Gutierrez Suarez
 */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <stdbool.h>

#ifndef DB_H
#define DB_H

/**
 * The available log events.
 */
typedef enum {
    LOG_NEW_CONNECTION,
    LOG_CLOSED_CONNECTION,
    LOG_TOO_COLD,
    LOG_TOO_HOT,
    LOG_INVALID_ID,
    LOG_NEW_DATA_FILE,
    LOG_DATA_INSERT,
    LOG_DATA_FILE_CLOSED,
    LOG_TIMEOUT
} log_codes;

/**
 * log_payload is a struct containing what we need to construct the log.
 */
typedef struct {
    log_codes code;
    sensor_id_t id;
    sensor_value_t data;
} log_payload;


/**
 * Initialize the Database.
 */
void *db_init();

/**
 * Adds an event to the database.
 */
void log_init();

/**
 * Closes the database and the file descriptor's write end.
 * Should only be called from the parent process.
 * @return
 */
int db_close();

/**
 * Adds an event to the pipe for logging.
 * @param code An enum with the possible log events.
 * @param data The data saved to the log.
 * @param id The id of the sensor.
 */
void log_pipe_write(log_codes code, sensor_id_t id, sensor_value_t data);

#endif //DB_H
