// Variant of tcp_client that reads fixed counts

#include <signal.h>     // sig_atomic_t
#include <stdio.h>      // printf
#include <time.h>       // time
#include <string.h>     // strlen
#include <assert.h>     // assert


#include "guard.h"
#include "sigterm_helper.hpp"
#include "tcp_event.h"
#include "tcp_read_write.h"
#include "tcp_read_buffer.hpp"


#define TIMEOUT_MS 1000

// Interrupt handler
volatile sig_atomic_t keep_running = 1;

void terminate(int signum) {
    printf("Received signal %d\n", signum);
    keep_running = 0;
}

bool a_second_has_just_passed(time_t* second_tracker) {
    if (*second_tracker == 0) {
        *second_tracker = time(NULL);
        return false;
    } else {
        // second_tracker > 0
        const time_t current_sec = time(NULL);
        if (current_sec != *second_tracker) {
            *second_tracker = current_sec;
            return true;
        } else {
            return false;
        }
    }
}

int main() {
    time_t time_tracker = (time_t)0;
    int writable_fd = 0;
    
    const char* hello = "Hello!";
    const size_t hello_size = strlen(hello) + 1;

    TcpReadBuffer buffer;
    tcp_read_buffer_init(&buffer, hello_size);

    register_sigint_handler(terminate);

    struct tcp_handler handler = tcp_handler_create();

    printf("Connected TCP for local fd %d\n", tcp_connect(&handler, NULL, 3333));
    for (; keep_running; ) {
        const struct tcp_event_result event = tcp_await_event(&handler, TIMEOUT_MS);
        switch (event.event) {
            case TCP_WRITABLE:
                writable_fd = event.fd;
                break;
            case TCP_HAS_DATA: {
                    enum TcpReadResult res = tcp_read_buffer_exact(event.fd, &buffer, hello_size);
                    switch (res) {
                        case TCP_READ_ERROR:
                            keep_running = false;
                            break;
                        case TCP_READ_COMPLETE:
                            assert(buffer.buf_start[buffer.buf_size - 1] == 0);
                            printf("Received \"%s\"\n", buffer.buf_start);
                            fflush(stdout);
                            tcp_read_buffer_empty(&buffer);
                            break;
                        case TCP_READ_INCOMPLETE:
                            printf("Incomplete read. Continue later.\n");
                            fflush(stdout);
                            break;
                    }
                }

                // print_bytes_from_fd(event.fd);
                break;
            case TCP_SHUTDOWN:
                // Note shutdown is informational only
                // note: when using await_event, the connection is already shutdown
                keep_running = 0;
                writable_fd = 0;
                break;
            default:
                ;
        }
        if (writable_fd && a_second_has_just_passed(&time_tracker)) {

            guard(tcp_send_surrogate(writable_fd, hello, hello_size), "Could not send\n");
            printf("Sent     \"%s\"\n", hello);
            fflush(stdout);
        }
    }

    tcp_handler_clear(&handler);
    tcp_read_buffer_free(&buffer);

    return 0;
}