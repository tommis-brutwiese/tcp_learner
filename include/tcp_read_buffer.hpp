
#ifndef TCP_READ_BUFFER_H
#define TCP_READ_BUFFER_H

#include "stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Buffer for simplified sequential reads
 * 
 */
struct TcpReadBuffer {
    unsigned char* buf_start; /* complete buffer */
    unsigned char* write_pos; /* pointer to write position within complete buffer */
    int buf_size; /* size of buffer */
};

/**
 * @brief Result for evaluating a call to read an exact number of bytes
 * 
 */
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
enum TcpReadResult tcp_read_buffer_exact(int sockfd, TcpReadBuffer* buf, size_t num_bytes);

/**
 * @brief Initializes a TcpReadBuffer for multiple sequential non-blocking reads
 * 
 * The TcpReadBuffer which holds the internal storage needs to be passed as a 
 * pointer. This method will then allocate the internal storage on the heap.
 * 
 * @param buf pointer to the structure which will hold the buffer.
 * @param siz number of bytes which will be available to the buffer
 */
void tcp_read_buffer_init(TcpReadBuffer* buf, size_t siz);

/**
 * @brief Frees the internal storage for the given TcpReadBuffer
 * 
 * Used memory will be freed.
 * 
 * @param buf 
 */
void tcp_read_buffer_free(TcpReadBuffer* buf);

/**
 * @brief Empty the read buffer
 * 
 * Memory remains allocated.
 * 
 * @param buf 
 */
void tcp_read_buffer_empty(TcpReadBuffer* buf);

#ifdef __cplusplus
}
#endif

#endif
