
#ifndef TCP_EVENT_H
#define TCP_EVENT_H

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

#include <sys/epoll.h>


#ifdef __cplusplus
extern "C" {
#endif


// #define TCP_DEBUG_EPOLL 1
#define MAX_EVENTS 10

typedef struct epoll_event epoll_event;

/* connection keeper for multiple connections */
struct tcp_handler {

    int epollfd;
    int event_next;   // next event to be returned to caller
    int event_count;  // current number of events

    epoll_event events[MAX_EVENTS];
};

// typedef struct tcp_handler tcp_handler;
//
// for now: leave additional enums active.
// later reduce to fewer (keep / discard)
//
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

struct tcp_handler tcp_handler_create();
void tcp_handler_clear(struct tcp_handler* handler);

int tcp_connect(struct tcp_handler* handler, char const * host, int port);
struct tcp_event_result tcp_await_event(struct tcp_handler* con, int timeout_ms);
int tcp_await_readable(struct tcp_handler* con, int fds, int timeout_ms);
int tcp_shutdown(struct tcp_handler* con, int fds);


#ifdef __cplusplus
}
#endif

#endif