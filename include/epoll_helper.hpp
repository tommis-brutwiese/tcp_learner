#ifndef EPOLL_HELPER_HPP
#define EPOLL_HELPER_HPP

// for now, use in c/cpp only

#include <sys/epoll.h>
#include <stdio.h>

#ifdef TCP_DEBUG_EPOLL

static void decipher_epoll_event(unsigned int event)
{
    if (event & EPOLLIN) {
        printf("EPOLLIN (available for read) |");
    }
    if (event & EPOLLOUT) {
        printf("EPOLLOUT (available for write) |");
    }
    if (event & EPOLLRDHUP) {
        printf("EPOLLRDHUP |");
    }
    if (event & EPOLLPRI) {
        printf("EPOLLPRI |");
    }
    if (event & EPOLLERR) {
        printf("EPOLLERR |");
    }
    if (event & EPOLLHUP) {
        printf("EPOLLHUP |");
    }
    printf("\n");
}
#endif


#endif