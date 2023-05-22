
#ifndef TCP_READ_WRITE_H
#define TCP_READ_WRITE_H

#ifdef __cplusplus
extern "C" {
#endif

/** Read at most numbytes from the given file-descriptor into the buffer
 * 
 * Forwards directly to `read`.
 */
int tcp_read_surrogate(int fd, void* buf, int numbytes);

/** Write numbytes from the buffer to the given file descriptor
 * 
 * Forwards directly to `send`.
 */
int tcp_send_surrogate(int fd, const void* buf, int numbytes);
int tcp_peek(int fd, char* buf, int bufsize);

#ifdef __cplusplus
}
#endif

#endif