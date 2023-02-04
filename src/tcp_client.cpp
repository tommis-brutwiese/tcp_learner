// Mainly copied from "man epoll" :-)

#include "sigterm_helper.hpp"
#include "epoll_helper.hpp"
#include "guard.hpp"

#include <unistd.h>             // read, sleep
#include <stdlib.h>             // EXIT_FAILURE
#include <stdio.h>              // println, perror
#include <string.h>             // memset
#include <netinet/in.h>         // sockaddr_in
#include <sys/socket.h>
#include <fcntl.h>              // fcntl
#include <sys/epoll.h>


#include <errno.h>              // EINTR
#include <assert.h>



volatile sig_atomic_t keep_running = 1;

#define CONNECT_PORT 3333
#define MAX_CONNECTIONS 2

// For the client, we want to send something, if we did not receive within timeout
#define TIMEOUT_MS 1000

void read_bytes_available(int sockfd) {
#define SIZ 10
    char in[SIZ + 1];  // one extra that will be 0
    memset(&in, 0, sizeof(in));
    int continue_read = 1;
    while (continue_read && keep_running) {
        continue_read = 0;  // will only continue later on if num_read == SIZE
        int num_read = read(sockfd, in, SIZ); // can be -1, 0, >0
        if (num_read > 0) {
            assert(in[sizeof(in) - 1] == 0);
            printf("%s", in);
        }
    }
}

void terminate(int signum) {
    printf("Received signal %d\n", signum);
    keep_running = 0;
}



int main()
{

    register_sigint_handler(terminate);

#define MAX_EVENTS 10
    struct epoll_event ev, events[MAX_EVENTS];
    int nfds, epollfd;

    /* Code to set up connecting socket, 'connect_sock' */
    struct sockaddr_in server_addr;

    int connect_sock =
        guard(socket(AF_INET, SOCK_STREAM, 0), "error opening socket\n");

    int can_write = 0;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(CONNECT_PORT);
    printf("Attempting to connect to port %d\n", CONNECT_PORT);


    guard(connect(connect_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)), "connect failed\n");
    printf("Connected to port %d\n", CONNECT_PORT);

    /* Now watch for incoming connections */
    epollfd = guard(epoll_create1(0), "epoll_create1\n");
    ev.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLET | EPOLLERR | EPOLLPRI | EPOLLHUP;
    ev.data.fd = connect_sock;
    guard(epoll_ctl(epollfd, EPOLL_CTL_ADD, connect_sock, &ev),
          "epoll_ctl: connect_sock\n");
    while (keep_running) {
        nfds = epoll_wait(epollfd, events, MAX_EVENTS, TIMEOUT_MS);
        if (nfds & EINTR) {
            keep_running = 0;
        } else if (nfds < 0) {
            guard(nfds, "epoll_wait\n");
        }

        if (!keep_running) {
            continue;
        }

        if (nfds > 0) {  // Read or close
            // printf("received nfds %d\n", nfds);
            for (int n = 0; n < nfds; ++n) {
                uint32_t event = events[n].events;
                int32_t fd = events[n].data.fd;
//                decipher_epoll_event(event);
//                printf("Event %d on fd %d\n", event, fd);


                if (event & EPOLLRDHUP) {
                    // event & EPOLLRDHUP: regularly
                    // otherwise: irregularly
                    // in any case: just hang up

                    shutdown(fd, SHUT_RDWR);  // ignore result
                    printf("Shutdown fd %d regularly\n", fd);
                    keep_running = 0;
                } else {
                    if (event & EPOLLIN) {
                        assert(fd == connect_sock);
                        read_bytes_available(fd);
                    }
                    if (event & EPOLLOUT) {
                        can_write = 1;
                    }
                }
            }
        }
        else
        {
            if (can_write) {
//                printf("timeout (%d ms)\n", TIMEOUT_MS);
                const char* hello = "Hello!\n";
                guard(send(connect_sock, hello, strlen(hello), 0), "Could not send\n");
                can_write = 0;  // Event will signal when we can write again
            }
        }
    }
    printf("Exit due to SIGTERM. SHOULD close all connections (not doing so now)\n");

    return 0;
}
