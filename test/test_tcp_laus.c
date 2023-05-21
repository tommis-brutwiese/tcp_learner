#include <assert.h>

#include "tcp_laus.h"

void test_tcp_handler_create() {
    struct tcp_handler handler = tcp_handler_create();

    assert(handler.event_count == 0);
    assert(handler.event_next == 0);
    assert(handler.epollfd != 0);  // must be set
}

int main() {
    test_tcp_handler_create();
    return 0;
}