/**
 * \author Nicolas Gutierrez Suarez
 */

#include <unistd.h>
#include <wait.h>
#include <pthread.h>
#include <limits.h>

#include "sensor_db.h"
#include "connmgr.h"
#include "sbuffer.h"
#include "datamgr.h"

int main(int argc, char *argv[]) {
    ERROR_HANDLER(argc != 2, "Wrong number of arguments.");

    long port = strtol(argv[1], NULL, 10);
    ERROR_HANDLER(port == LONG_MAX || port == LONG_MIN, "Error parsing port.");
    DEBUG_PRINTF("Port Selected: %li", port);

    log_init(); // Start the logger, the parent process will continue execution here.
    sbuffer_init(); // Start the buffer.

    // Create 3 threads for each part of the server. Join them to wait until all of them terminate.
    pthread_t tid[3];
    pthread_create(&tid[0], NULL,connmgr_startup, (void *) &port);
    pthread_create(&tid[1], NULL, datamgr_init, NULL);
    pthread_create(&tid[2], NULL, db_init, NULL);

    for (int i = 0; i < 3; ++i) {
        pthread_join(tid[i], NULL);
    }

    // When the database is closed, the child process will manage to terminate.
    ERROR_HANDLER(db_close() != 0, "DB closed improperly.");
    wait(NULL);

    sbuffer_free();
}
