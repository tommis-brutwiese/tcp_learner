#pragma once

#include <sys/epoll.h>

void decipher_epoll_event(unsigned int event)
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
