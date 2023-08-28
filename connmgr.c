/**
 * \author Nicolas Gutierrez Suarez
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "config.h"
#include "connmgr.h"
#include "sensor_db.h"
#include "lib/tcpsock.h"
#include "sbuffer.h"

/**
 * Listens for data and inserts it into the shared buffer. Logs the events.
 * @param _client a tpsock_t client sending data.
 * @return NULL
 */
static void *connmgr_socket_start(void *_client) {
    tcpsock_t *client = (tcpsock_t *) _client; // Client passed as void pointer in pthread_create.

    int bytes; // Number of bytes read from the socket.

    bool is_logged = false; // State variable to only log the connection once.
    sensor_id_t id = 0; // Saving this id so if the connection stops, the id persists.

    DEBUG_PRINTF("Connection started: %lu", pthread_self());

    do {
        int result; // Save the result of tcp_receive.

        sensor_data_t *data = malloc(sizeof(sensor_data_t));
        memset(data, 0, sizeof(sensor_data_t)); // Set the data to 0 so valgrind does not complain.

        // Read the ID
        bytes = sizeof(sensor_id_t);
        result = tcp_receive(client, (void *) &data->id, &bytes, DTIMEOUT);
        ERROR_HANDLER(result == TCP_SOCKET_ERROR, "Error reading TCP data.");

        if (!is_logged) {
            // This runs once, when the connection begins, and logs the ID of the sensor connected.
            log_pipe_write(LOG_NEW_CONNECTION, data->id, 0);
            id = data->id;
            is_logged = true;
        }

        // Read temperature
        bytes = sizeof(data->value);
        result = tcp_receive(client, (void *) &data->value, &bytes, DTIMEOUT);
        ERROR_HANDLER(result == TCP_SOCKET_ERROR, "Error reading TCP data.");

        // Read timestamp
        bytes = sizeof(data->ts);
        result = tcp_receive(client, (void *) &data->ts, &bytes, DTIMEOUT);

        // Three distinct cases, if the client disconnects, we log it together with its ID.
        if (result == TCP_CONNECTION_CLOSED) {
            DEBUG_PRINTF("Peer has closed connection.");
            log_pipe_write(LOG_CLOSED_CONNECTION, id, 0);
            break;
        }
        // By setting SO_RCVTIMEO to the DTIMEOUT set in the preprocessor, if the client takes longer than
        // DTIMEOUT, the socket sets errno to EAGAIN.
        else if (errno == EAGAIN) {
            DEBUG_PRINTF("Peer has timed-out.");
            log_pipe_write(LOG_TIMEOUT, id, 0);
            break;
        }
        // If the bytes != 0 and result is not an error then we can insert that data to the buffer.
        else if ((result == TCP_NO_ERROR) && bytes) {
            DEBUG_PRINTF("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld",
                         data->id, data->value, (long int) data->ts);
            sbuffer_insert(data);
        }
    } while (1);

    tcp_close(&client);
    return NULL;
}

void *connmgr_startup(void *port) {
    pthread_t tid[D_MAX_CONN];
    tcpsock_t *server, *client;
    int conn_counter = 0;

    DEBUG_PRINTF("Server startup. %lu", pthread_self());
    ERROR_HANDLER((tcp_passive_open(&server, *((int *) port)) != TCP_NO_ERROR), "Error opening TCP connection.");
    do {

        ERROR_HANDLER(tcp_wait_for_connection(server, &client) != TCP_NO_ERROR, "Error connecting to client");
        DEBUG_PRINTF("Incoming client connection.");

        // Start a new thread whenever there is a new incoming connection. Stop when the connection counter == D_MAX_CONN
        pthread_create(&tid[conn_counter], NULL, connmgr_socket_start, client);
        conn_counter++;
    } while (conn_counter < D_MAX_CONN);

    // Wait for all the clients to finish.
    for (int i = 0; i < D_MAX_CONN; ++i) {
        pthread_join(tid[i], NULL);
    }
    ERROR_HANDLER(tcp_close(&server) != TCP_NO_ERROR, "Error closing TCP server.");

    DEBUG_PRINTF("Server is shutting down.");

    // Insert an EOF marker to the buffer.
    sensor_data_t *data = malloc(sizeof(sensor_data_t));
    memset(data, 0, sizeof(sensor_data_t));
    sbuffer_insert(data);

    pthread_exit(NULL);
}