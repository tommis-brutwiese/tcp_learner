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

char const* get_sample_message(int n) {
    switch (n % 3) {
        case 0:  return "Hello!";
        case 1:  return "Yeah";
        case 2:  return "A slightly longer string";
        default: return "";
    }
}

void calculate_read_now_and_remaining(int* read_now, int* remaining, int buf_size) {
    if (*remaining > buf_size) {
        *read_now = buf_size;
        *remaining -= buf_size;
    } else {
        *read_now = *remaining;
        *remaining = 0;
    }
}

void read_and_process_incoming_data(int fd, TcpReadBuffer* buffer, int remaining_to_read) {
    int continue_reading = true;

    while (continue_reading) {
        
        int read_now = 0;
        calculate_read_now_and_remaining(&read_now, &remaining_to_read, buffer->buf_size);
        fflush(stdout);

        if (remaining_to_read == 0) {
            continue_reading = false;
        }

        enum TcpReadResult res = tcp_read_buffer_exact(fd, buffer, read_now);
        switch (res) {
            case TCP_READ_ERROR:
                keep_running = false;
                break;
            case TCP_READ_COMPLETE:
                //assert(buffer.buf_start[buffer.buf_size - 1] == 0);
                printf("Received \"%-12s\"  read now: %2d  remaining: %2d\n", buffer->buf_start, read_now, remaining_to_read);
                fflush(stdout);
                tcp_read_buffer_empty(buffer);
                break;
            case TCP_READ_INCOMPLETE:
                printf("                            Incomplete read. Continue later.\n");
                fflush(stdout);
                break;
        }
    }
}

int main() {
    register_sigint_handler(terminate);
    const int buf_size = 10;

    time_t time_tracker = (time_t)0;
    int writable_fd = 0;
    int last_msg_siz = 0;
    int cnt = 0;
    
    printf("Initializing buffer...\n");
    TcpReadBuffer buffer;
    tcp_read_buffer_init(&buffer, buf_size);

    struct tcp_handler handler = tcp_handler_create();
    printf("Connected TCP for local fd %d\n", tcp_connect(&handler, NULL, 3333));

    for (; keep_running; ) {
        const struct tcp_event_result event = tcp_await_event(&handler, TIMEOUT_MS);
        switch (event.event) {
            case TCP_WRITABLE:
                writable_fd = event.fd;
                break;
            case TCP_HAS_DATA: {
                read_and_process_incoming_data(event.fd, &buffer, last_msg_siz);
                }
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
            char const* msg = get_sample_message(cnt);
            cnt++;

            last_msg_siz = strlen(msg) + 1;

            guard(tcp_send_surrogate(writable_fd, msg, last_msg_siz), "Could not send\n");
            printf("Sent     \"%s\"\n", msg);
            fflush(stdout);
        }
    }

    tcp_handler_clear(&handler);
    tcp_read_buffer_free(&buffer);

    return 0;
}