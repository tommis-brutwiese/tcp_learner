// Variant of tcp_client that reads c-strings

#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <string.h>

#include "guard.h"
#include "sigterm_helper.hpp"
#include "tcp_event.h"
#include "tcp_read_write.h"

void read_bytes_available(int sockfd);



#define TIMEOUT_MS 1000

// Interrupt handler
volatile sig_atomic_t keep_running = 1;

void terminate(int signum) {
    printf("Received signal %d\n", signum);
    keep_running = 0;
}

void print_bytes_from_fd(int sockfd) {
    // consciously small buffer to provoke multiple reads
#define SIZ 10
    char in[SIZ + 1];  // one extra that will be 0
    memset(&in, 0, sizeof(in));
    int continue_read = 1;
    while (continue_read && keep_running) {
        continue_read = 0;  // will only continue later on if num_read == SIZE
        int num_read = tcp_read_surrogate(sockfd, in, SIZ); // can be -1, 0, >0
        if (num_read > 0) {
            assert(in[sizeof(in) - 1] == 0);
            printf("Received \"%s\"\n", in);
            fflush(stdout);

            if (num_read == SIZ) {
                continue_read = 1;
            }
        }
    }
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
    register_sigint_handler(terminate);

    struct tcp_handler handler = tcp_handler_create();

    printf("Connected TCP for local fd %d\n", tcp_connect(&handler, NULL, 3333));
    for (; keep_running; ) {
        const struct tcp_event_result event = tcp_await_event(&handler, TIMEOUT_MS);
        switch (event.event) {
            case TCP_WRITABLE:
                writable_fd = event.fd;
                break;
            case TCP_HAS_DATA:
                print_bytes_from_fd(event.fd);
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

            const char* hello = "Hello!";
            guard(tcp_send_surrogate(writable_fd, hello, strlen(hello) + 1), "Could not send\n");
            printf("Sent     \"%s\"\n", hello);
            fflush(stdout);
        }
    }

    tcp_handler_clear(&handler);
    return 0;
}