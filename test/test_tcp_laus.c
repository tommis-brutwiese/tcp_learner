#include <assert.h>

#include "tcp_laus.h"

void test_tcp_handler_create() {
    struct tcp_handler handler = tcp_handler_create();
    assert(handler.epollfd == 0);
    assert(handler.event_count == 0);
    assert(handler.event_next == 0);
    for (int x = 0; x != MAX_EVENTS; ++x) {
        // never mind
        // assert(handler.events[x] == ONLY ZEROES);
    }
    /*
    for (int i = 0; i != MAX_FDS; ++i) {
        assert(handler.fds[i] == 0);
    }
    assert(handler.fds_count == 0);
    */
}

int main() {
    test_tcp_handler_create();
    return 0;
}