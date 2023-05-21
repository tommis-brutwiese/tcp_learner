// Variant of tcp_client that reads fixed counts

#include <signal.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "sigterm_helper.hpp"
#include "tcp_laus.h"

void read_bytes_available(int sockfd);



#define TIMEOUT_MS 1000

// Interrupt handler
volatile sig_atomic_t keep_running = 1;

void terminate(int signum) {
    printf("Received signal %d\n", signum);
    keep_running = 0;
}

struct TcpReadBuffer {
    unsigned char* buf_start; /* complete buffer */
    unsigned char* write_pos; /* pointer to write position within complete buffer */
    int buf_size; /* size of buffer */
};

static void tcp_read_buffer_zero(TcpReadBuffer* buf) {
    memset(buf, 0, sizeof(TcpReadBuffer));
    assert(buf->buf_start == NULL);
    assert(buf->write_pos == NULL);
    assert(buf->buf_size == 0);
}

void tcp_read_buffer_init(TcpReadBuffer* buf, size_t siz) {
    assert(buf);
    tcp_read_buffer_zero(buf);
    assert(buf->buf_start = (unsigned char*)calloc(siz, 1));
    buf->write_pos = buf->buf_start;
}

void tcp_read_buffer_clear(TcpReadBuffer* buf) {
    assert(buf);
    free(buf->buf_start);
    tcp_read_buffer_zero(buf);
}

void tcp_read_buffer_reset(TcpReadBuffer* buf) {
    assert(buf);
    buf->write_pos = buf->buf_start;
}


enum TcpReadResult {
    TCP_READ_COMPLETE = 0,
    TCP_READ_INCOMPLETE,
    TCP_READ_ERROR
};

/**
 * @brief Reads exactly num bytes into the given buffer
 * 
 * Returns if the desired total number could be read.
 * The intention of this method is that it can be called multiple
 * times non-blocking, filling up the read bytes as they come, until
 * the total desired number could be read.
 * 
 * The desired number must fit.
 * 
 * @param buf 
 * @param num_bytes 
 * @return enum TcpReadResult 
 */
enum TcpReadResult tcp_read_buffer_exact(int sockfd, TcpReadBuffer* buf, size_t num_bytes) {
    assert(buf);
    assert(buf->buf_start);
    assert(buf->write_pos);

    if (buf->write_pos > buf->buf_start + num_bytes) {
        /* actually only valid: end pos + 1 */
        return TCP_READ_COMPLETE;
    }

    const int num_read_previously = buf->write_pos - buf->buf_start;
    const int remaining = num_bytes - num_read_previously;
    assert(remaining > 0);

    const int num_read = tcp_read_surrogate(sockfd, buf->write_pos, remaining);
    if (num_read == -1) {
        return TCP_READ_ERROR;
    }

    buf->write_pos += num_read;
    assert(buf->write_pos >= buf->buf_start);

    const size_t total_read = buf->write_pos - buf->buf_start;
    if (total_read < num_bytes) {
        return TCP_READ_INCOMPLETE;
    } else if (total_read == num_bytes) {
        return TCP_READ_COMPLETE;
    } else {
        return TCP_READ_ERROR;
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
                            tcp_read_buffer_reset(&buffer);
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
    tcp_read_buffer_clear(&buffer);

    return 0;
}