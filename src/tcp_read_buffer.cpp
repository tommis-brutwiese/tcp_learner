#include "tcp_read_buffer.hpp"

#include "tcp_read_write.h"

#include "assert.h"
#include <cstring>




static bool tcp_read_buffer_is_init(TcpReadBuffer const* buf) {
    return buf && buf->buf_start && buf->buf_size; 
}

/**
 * @brief reset all elements of the read buffer to zero
 * 
 * @param buf 
 */
static void tcp_read_buffer_zero_internal(TcpReadBuffer* buf) {
    memset(buf, 0, sizeof(TcpReadBuffer));
    assert(buf->buf_start == NULL);
    assert(buf->write_pos == NULL);
    assert(buf->buf_size == 0);
}

static void tcp_read_buffer_reset_internal(TcpReadBuffer* buf, unsigned char* buf_start, size_t buf_size) {
    assert(buf);
    tcp_read_buffer_zero_internal(buf);

    buf->buf_size = buf_size;
    buf->buf_start = buf_start;
    buf->write_pos = buf->buf_start;
}

void tcp_read_buffer_empty(TcpReadBuffer* buf) {
    assert(tcp_read_buffer_is_init(buf));
    tcp_read_buffer_reset_internal(buf, buf->buf_start, buf->buf_size);
}




void tcp_read_buffer_init(TcpReadBuffer* buf, size_t siz) {
    assert(buf);
    unsigned char* buf_start = NULL;
    assert(buf_start = (unsigned char*)calloc(siz, 1));
    tcp_read_buffer_reset_internal(buf, buf_start, siz);
}

void tcp_read_buffer_free(TcpReadBuffer* buf) {
    assert(buf);
    free(buf->buf_start);
    tcp_read_buffer_zero_internal(buf);
}

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