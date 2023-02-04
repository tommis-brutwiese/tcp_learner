// Mainly copied from "man epoll" :-)

#include "sigterm_helper.hpp"
#include "epoll_helper.hpp"
#include "guard.hpp"

#include <unistd.h>             // read
#include <stdlib.h>             // EXIT_FAILURE
#include <stdio.h>              // println, perror
#include <string.h>             // memset
#include <netinet/in.h>         // sockaddr_in
#include <sys/socket.h>
#include <fcntl.h>              // fcntl
#include <sys/epoll.h>
#include <errno.h>              // EINTR


// Desire:
// server(handle_accept, handle_shutdown, handle_read, handle_write)
//
// ... and that server shall also handle SIGTERM correctly (shut down cleanly)

void setnonblocking(int socket)
{
    guard(fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK),
          "error setting accepted connection to nonblocking\n");
}

volatile sig_atomic_t keep_running = 1;

#define LISTEN_PORT 3333
#define MAX_CONNECTIONS 2
#define TIMEOUT_MS 10

void echo_bytes_available(int sockfd) {
#define SIZ 10
    char in[SIZ];
    int continue_read = 1;
    while (continue_read && keep_running) {
        continue_read = 0;  // will only continue later on if num_read == SIZE
        int num_read = read(sockfd, in, SIZ); // can be -1, 0, >0
        if (num_read > 0) {
            if (num_read == SIZ) {
                continue_read = 1;
            }

            int num_sent = send(sockfd, in, num_read, 0);  // can be -1, 0, >0
            if (num_sent == num_read) {
                // printf("Echoed %d bytes\n", num_sent);
            } else {
                // ... and desire close?
            }
        } else if (num_read == 0) {
            // hmm... retry later
        } else {  // num_read < 0
            // ... and desire close?
        }
    }
}

void terminate(int signum) {
    printf("Received signal %d\n", signum);
    keep_running = 0;
}

int listen_on_socket(int listen_port) {
    int listen_sock =
        guard(socket(AF_INET, SOCK_STREAM, 0), "error opening socket\n");

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(listen_port);
    printf("Attempting to listen on port %d\n", listen_port);

    guard(bind
          (listen_sock, (struct sockaddr *) &server_addr,
           sizeof(server_addr)), "bind to port failed (in use?)\n");
    guard(listen(listen_sock, MAX_CONNECTIONS), "listen failed\n");
    printf("Listening on port %d\n", listen_port);
    return listen_sock;
}


int accept_connection(int listen_sock)
{
    int conn_sock;
    unsigned int socklen = 0;
    struct sockaddr_in cli_addr;

    conn_sock = guard(accept(listen_sock,
                                (struct sockaddr *) &cli_addr,
                                &socklen), "accept\n");
    setnonblocking(conn_sock);
    printf("Connected to fd %d\n", conn_sock);
    return conn_sock;
}

void subscribe_to_listen_sock(int epollfd, int listen_sock) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    guard(epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev),
          "epoll_ctl: listen_sock\n");
}

void subscribe_to_accepted_connection(int epollfd, int conn_sock) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    ev.data.fd = conn_sock;
    guard(epoll_ctl(epollfd, EPOLL_CTL_ADD, conn_sock, &ev),
          "epoll_ctl: listen_sock\n");
}

void unsubscribe_from_accepted_connection(int epollfd, int fd) {
    struct epoll_event ev;
    guard(epoll_ctl(epollfd, EPOLL_CTL_DEL, fd,
                &ev), "epoll_ctl: fd\n");
}

int main()
{
    register_sigint_handler(terminate);

#define MAX_EVENTS 10
    struct epoll_event events[MAX_EVENTS];
    int nfds, epollfd;

    /* Code to set up listening socket, 'listen_sock' */
    int listen_sock = listen_on_socket(LISTEN_PORT);
    int accepted_sock;


    /* Now watch for incoming connections */
    epollfd = guard(epoll_create1(0), "epoll_create1\n");
    subscribe_to_listen_sock(epollfd, listen_sock);

    while (keep_running) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, TIMEOUT_MS);
        if (nfds & EINTR) {
            keep_running = 0;
            // Should hangup on all connections?
        } else if (nfds < 0) {
            guard(nfds, "epoll_wait\n");
        }

        if (!keep_running) {
            continue;
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                // Handle new incoming connection
                accepted_sock = accept_connection(listen_sock);
                subscribe_to_accepted_connection(epollfd, accepted_sock);
            } else {
                // Handle existing connection
                uint32_t event = events[n].events;
                int32_t fd = events[n].data.fd;

                if (event == EPOLLIN) {
                    echo_bytes_available(events[n].data.fd);
                } else {
                    // event & EPOLLRDHUP: regularly
                    // otherwise: irregularly
                    // in any case: just hang up

                    shutdown(fd, SHUT_RDWR);  // ignore result
                    printf("Shutdown fd %d regularly\n", fd);
                    unsubscribe_from_accepted_connection(epollfd, fd);
               }
            }
        }
    }
    printf("Exit due to SIGTERM. SHOULD close all connections (not doing so now)\n");

    return 0;
}
