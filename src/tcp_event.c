// using this before including tcp_event.h - dep on order, not nice
// #define TCP_DEBUG_EPOLL

#include "tcp_event.h"
#include "guard.h"

#include <stdio.h>       // printf
#include <errno.h>       // EINTR
#include <assert.h>      // assert
#include <string.h>      // memset
#include <sys/socket.h>  // socket
#include <netinet/in.h>  // sockaddr_in




/** clear handler, but do not close handles
*/
static void tcp_handler_set_zero(struct tcp_handler* handler) {
    memset(handler, 0, sizeof(struct tcp_handler));

    assert(handler->event_count == 0);
    assert(handler->event_next == 0);
    assert(handler->epollfd == 0);
}

struct tcp_handler tcp_handler_create() {
    struct tcp_handler handler;
    tcp_handler_set_zero(&handler);
    handler.epollfd = guard(epoll_create1(0), "epoll_create1\n");
    return handler;
}

void tcp_handler_clear(struct tcp_handler* handler) {
    tcp_handler_set_zero(handler);

    // tcp_shutdown(handler, handler->)
}

int tcp_connect(struct tcp_handler* handler, char const * host, int port) {
    assert(handler != NULL);

    // create socket file-descriptor
    int connect_sock = guard(socket(AF_INET, SOCK_STREAM, 0), "error opening socket\n");

    // connnect fd to tcp-socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;  // um.. where is host?
    server_addr.sin_port = htons(port);
    printf("Attempting to connect to port %d\n", port);

    guard(connect(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)), "connect failed\n");
    printf("Connected to port %d\n", port);

    /* Now watch for incoming connections */
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET | EPOLLERR | EPOLLPRI | EPOLLHUP;
    ev.data.fd = connect_sock;
    guard(epoll_ctl(handler->epollfd, EPOLL_CTL_ADD, connect_sock, &ev),
          "epoll_ctl: connect_sock\n");

    return connect_sock;
}

enum en_tcp_result_internal {
    TCPINT_GENERIC_INTR,
    TCPINT_EVENTS_OR_TIMEOUT,
    TCPINT_ERROR
    // could have an error
};

/* @return 0 if okay or -1 on error */
static enum en_tcp_result_internal tcp_wait_for_events (struct tcp_handler* con, int timeout_ms) {
    // this internal method shall only be called if all previous events have been processed
    assert(con->event_count == 0);

    con->event_next = 0;
    con->event_count = epoll_wait(con->epollfd, con->events, MAX_EVENTS, timeout_ms);
    
    if ((con->event_count >= 0)) {
        return TCPINT_EVENTS_OR_TIMEOUT;
    } else {
        // error case
        if (errno & EINTR) {
            // timeout or interrupt before timeout expired
            con->event_count = 0;
            return TCPINT_GENERIC_INTR;
        } else {
            // must be an error
            guard(errno, "epoll_wait\n");
            return TCPINT_ERROR;
        }
    }
}

static int tcp_has_event(struct tcp_handler *con) {
    return con->event_next < con->event_count;
}

static struct tcp_event_result tcp_retrieve_event(struct tcp_handler* con) {
    assert(tcp_has_event(con));

    if (con->event_count == 0 || (con->event_next >= con->event_count)) {
        struct tcp_event_result ev = {TCP_NO_DATA, 0};
        return ev;
    }

    uint32_t event = con->events[con->event_next].events;
    int32_t fd = con->events[con->event_next].data.fd;    

#ifdef TCP_DEBUG_EPOLL
    decipher_epoll_event(event);
    printf("Event %d on fd %d\n", event, fd);
#endif

    con->event_next++;

    // if last event, clear list
    //
    // RATHER: clear struct with information via clear method
    if (con->event_next == con->event_count) {
        con->event_next = con->event_count = 0;
    }

    // TODO: what to do with multiple bits?
    // Perhaps return TCP_THINK_THIS_THROUGH?

    if (event & EPOLLRDHUP) {
        // event & EPOLLRDHUP: regularly
        // otherwise: irregularly
        // in any case: just hang up

        shutdown(fd, SHUT_RDWR);  // ignore result
        struct tcp_event_result ev = {TCP_SHUTDOWN, fd};
        return ev;

    } else {
        if (event & EPOLLIN) {
            struct tcp_event_result ev = {TCP_HAS_DATA, fd};
            return ev;
        } else if (event & EPOLLOUT) {
            struct tcp_event_result ev = {TCP_WRITABLE, fd};
            return ev;
        } else {
            struct tcp_event_result ev = {TCP_NO_DATA, 0};
            return ev;
        }
    }
}

struct tcp_event_result tcp_await_event(struct tcp_handler *con, int timeout_ms) {
    // Wait for new events
    // OR
    // Return next unprocessed one

    if (!tcp_has_event(con)) {
        enum en_tcp_result_internal internal_res = tcp_wait_for_events(con, timeout_ms);

        switch (internal_res) {
            case TCPINT_ERROR: {
                    assert(0);  // die
                    struct tcp_event_result res = {TCP_OTHER_EVENT, 0};
                    return res;
                }

            case TCPINT_GENERIC_INTR: {
                    // special case interrupt: event list is empty, return
                    struct tcp_event_result res = {TCP_GENERIC_INTR, 0};
                    return res;
                }                

            case TCPINT_EVENTS_OR_TIMEOUT:
                // regular case
                break;
        } 
    } 

    if (tcp_has_event(con)) {
        // This will copy the struct. That is okay for me.
        return tcp_retrieve_event(con);
    } else {
        struct tcp_event_result ev = {TCP_NO_DATA, 0};
        return ev;
    }
}

