#include "tcp_read_write.h"

#include "unistd.h"      // for "read"
#include "sys/socket.h"  // for "send"

int tcp_read_surrogate(int fd, void* buf, int numbytes) {
    return read(fd, buf, numbytes);
}

int tcp_send_surrogate(int fd, const void* buf, int numbytes) {
    return send(fd, buf, numbytes, 0);
}
