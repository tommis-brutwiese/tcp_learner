
#ifndef TCP_LAUS_H
#define TCP_LAUS_H

/** TCP learner
 * 
 * Attempt to provide a shell around the basic tcp functionality, in order to
 * 
 * * Simple, SAFE and consistent usage for basic usecases
 * * Make looking up the tech behind the simple use easy
 * * Make testing different techniques easy
 * 
 * Safe: avoid hung file-descriptors - and waiting for events easy.
 * 
 * No-Goals:
 * 
 * * compatibility is not important
 */

#include "epoll_helper.hpp"

#include <unistd.h>             // read, sleep
#include <stdlib.h>             // EXIT_FAILURE
#include <stdio.h>              // println, perror
#include <string.h>             // memset
#include <netinet/in.h>         // sockaddr_in
#include <sys/socket.h>
#include <fcntl.h>              // fcntl
#include <sys/epoll.h>

#ifdef __cplusplus
extern "C" {
#endif

//

// #define TCP_DEBUG_EPOLL 1
#define MAX_EVENTS 10
#define MAX_FDS 2

typedef struct epoll_event epoll_event;

/* connection keeper for multiple connections */
struct tcp_handler {
    // int port;
    // int connect_socket;  // socket connected to

    // int fds[MAX_FDS];
    // int fds_count;

    int epollfd;
    int event_next;   // next event to be returned to caller
    int event_count;  // current number of events

    epoll_event events[MAX_EVENTS];
};
// typedef struct tcp_handler tcp_handler;
//
// for now: leave additional enums active.
// later reduce to fewer (keep / discard)

enum en_tcp_event {
    TCP_NO_DATA,   // keep
    TCP_HAS_DATA,  // keep
    TCP_WRITABLE,  // discard
    TCP_SHUTDOWN,  // informational: socket has already been shutdown
    TCP_GENERIC_INTR,  // discard (NO_DATA)
    TCP_OTHER_EVENT  // discard (NO_DATA)
};



struct tcp_event_result {
    enum en_tcp_event event;  // event that occurred
    int fd;  // file descriptor for event (if given)
};

// next: modify existing code to work again
// step by step

struct tcp_handler tcp_handler_create();
void tcp_handler_clear(struct tcp_handler* handler);

int tcp_connect(struct tcp_handler* handler, char const * host, int port);
struct tcp_event_result tcp_await_event(struct tcp_handler* con, int timeout_ms);
int tcp_await_readable(struct tcp_handler* con, int fds, int timeout_ms);
int tcp_shutdown(struct tcp_handler* con, int fds);

/** Read at most numbytes from the given file-descriptor into the buffer
 * 
 * Forwards directly to `read`.
 */
int tcp_read_surrogate(int fd, void* buf, int numbytes);

/** Write numbytes from the buffer to the given file descriptor
 * 
 * Forwards directly to `send`.
 */
int tcp_send_surrogate(int fd, const void* buf, int numbytes);
int tcp_peek(int fd, char* buf, int bufsize);



#ifdef __cplusplus
}
#endif

#endif