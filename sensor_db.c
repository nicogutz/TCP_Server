/**
 * \author Nicolas Gutierrez Suarez
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "sensor_db.h"
#include "sbuffer.h"
#include <inttypes.h>

#define LOG_FILE_NAME "gateway.log"
#define DB_FILE_NAME "data.csv"

#define READ_END 0
#define WRITE_END 1

FILE *log_file, *db_file; // The pointer to the file stream for the log and the DB.
int fd[2]; // The file descriptor for the pipe.

/**
 * This function inserts a new line into the log file. It flushes the stream to make sure the data is written
 * on the fly in case there is a signal that ends the process before closing the file.
 * @param id The sensor ID
 * @param value The reported value of the sensor.
 * @param ts The timestamp of the datum.
 * @return Number of characters written, negative if there is an error.
 */
static int insert_sensor( sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    int res = fprintf(db_file, "%"PRIu16",%lf,%li\n", id, value, ts);
    fflush(db_file);
    return res;
}

int db_close() {
    return close(fd[WRITE_END]); // Important to let the child die.
}

void *db_init() {
    db_file = fopen(DB_FILE_NAME, "w");
    ERROR_HANDLER(db_file == NULL, "File creation did not work.");
    log_pipe_write(LOG_NEW_DATA_FILE, 0, 0);

    sbuffer_node_t *node = NULL;
    sensor_data_t *data;
    do {
        // Same idea as with the datamgr, read until the EOF is sent. Insert all data into the database, write the
        // log as required.
        int res;
        do {
            res = sbuffer_read(&node, &data);
            usleep(10);
        } while (res == SBUFFER_NO_DATA);
        if (data->id == 0) break;
        DEBUG_PRINTF("Datum read: %i %f %li", data->id, data->value, data->ts);
        ERROR_HANDLER(insert_sensor(data->id, data->value, data->ts) < 0, "Error writing to file.");
        log_pipe_write(LOG_DATA_INSERT, data->id, 0);
    } while (1);

    ERROR_HANDLER(fclose(db_file) != 0, "Error closing DB");
    log_pipe_write(LOG_DATA_FILE_CLOSED, 0, 0);
    pthread_exit(NULL);
}

void log_pipe_write(log_codes code, sensor_id_t id, sensor_value_t data) {
    log_payload payload;
    memset(&payload, 0, sizeof(log_payload)); // This avoids unsafe behavior due to padding.
    payload.id = id;
    payload.data = data;
    payload.code = code;
    write(fd[WRITE_END], &payload, sizeof(log_payload));
}

static void log_child_process() {
    close(fd[WRITE_END]);
    ssize_t n;
    log_payload payload;
    int last_log = 0;
    while ((n = read(fd[READ_END], &payload, sizeof(payload))) > 0) {
        // We read the stream until the EOF is reached (when the parent closes the WRITE_END pipe).
        // Also count how many logs have been generated so far.
        last_log++;
        fprintf(log_file, "%i %lu ", last_log, (unsigned long) time(NULL));
        switch (payload.code) {
            case LOG_NEW_CONNECTION:
                fprintf(log_file, "Sensor node %i has opened a new connection.\n", payload.id);
                break;
            case LOG_CLOSED_CONNECTION:
                fprintf(log_file, "Sensor node %i has closed the connection.\n", payload.id);
                break;
            case LOG_TIMEOUT:
                fprintf(log_file, "Sensor node %i has timed-out.\n", payload.id);
                break;
            case LOG_TOO_COLD:
                fprintf(log_file, "Sensor node %i reports it’s too cold (avg temp = %lf).\n", payload.id,
                        payload.data);
                break;
            case LOG_TOO_HOT:
                fprintf(log_file, "Sensor node %i reports it’s too hot (avg temp = %lf).\n", payload.id,
                        payload.data);
                break;
            case LOG_INVALID_ID:
                fprintf(log_file, "Received sensor data with invalid sensor node ID %i.\n", payload.id);
                break;
            case LOG_NEW_DATA_FILE:
                fprintf(log_file, "A new data.csv file has been created.\n");
                break;
            case LOG_DATA_INSERT:
                fprintf(log_file, "Data insertion from sensor %i succeeded.\n", payload.id);
                break;
            case LOG_DATA_FILE_CLOSED:
                fprintf(log_file, "The data.csv file has been closed.\n");
                break;
        }
        fflush(log_file);
    }

    ERROR_HANDLER(n == -1 || close(fd[READ_END]) == -1 || fclose(log_file) != 0, "Error stopping logger.");
    _exit(EXIT_SUCCESS);
}

void log_init() {
    // We fork the main thread and initialize the log file only in the child (the parent has no use for it)
    ERROR_HANDLER(pipe(fd) == -1, "Pipe creation unsuccessful.");

    pid_t pid = fork();
    ERROR_HANDLER(pid < 0, "Fork was unsuccessful.");

    if (pid != 0) {
        close(fd[READ_END]);
        DEBUG_PRINTF("Parent process %i.", getpid());
        return;
    } else {
        log_file = fopen(LOG_FILE_NAME, "w");
        ERROR_HANDLER(log_file == NULL, "Error creating log file.");
    }
    DEBUG_PRINTF("Child process %i and log file created successfully.", getpid());
    log_child_process();
}



