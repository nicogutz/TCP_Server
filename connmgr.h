//
// Created by nicogusuarez on 26/12/22.
//

#ifndef CONNMGR_H
#define CONNMGR_H

#ifndef DTIMEOUT
#define DTIMEOUT 5
#endif

#ifndef D_MAX_CONN
#define D_MAX_CONN 3  // Maximum number of connections the server will handle before exiting.
#endif

/**
 * This is the main function handling TCP connections to the server. It makes a new thread for each incoming connection
 * under D_MAX_CONN. The port is set using a command line argument.
 * @param port Chosen port to open TCP
 * @return NULL
 */
void *connmgr_startup(void *port);
#endif //CONNMGR_H
