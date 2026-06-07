#ifndef MX_SOCKET_H
#define MX_SOCKET_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
typedef struct {
    int sockfd;
    socklen_t addrlen;
    bool blocking;
} MXSocket;

[[nodiscard]] bool mx_socket_init(MXSocket *s);
[[nodiscard]] bool mx_socket_listen(MXSocket *s, const char *port, int backlog);
[[nodiscard]] bool mx_socket_unix_listen(MXSocket *s, const char *path, int backlog);
[[nodiscard]] bool mx_socket_accept(const MXSocket *input, MXSocket *output);
void mx_socket_close(MXSocket *s);
[[nodiscard]] bool mx_socket_set_blocking(MXSocket *s, bool state);
[[nodiscard]] bool mx_socket_connect(MXSocket *s, const char *host, const char *port, int type);
[[nodiscard]] bool mx_socket_unix_connect(MXSocket *s, const char *path);
ssize_t mx_socket_read(MXSocket *s, void *data, size_t len, int flags);
ssize_t mx_socket_send(MXSocket *s, const void *data, size_t len, int flags);
[[nodiscard]] bool mx_socket_valid(const MXSocket *s);
[[nodiscard]] bool mx_socket_is_open(const MXSocket *s);
[[nodiscard]] bool mx_socket_readline(MXSocket *s, char **buffer, size_t *len);
ssize_t mx_socket_read_all(MXSocket *sock, void *buf, size_t bytes);
ssize_t mx_socket_write_all(MXSocket *sock, const void *buf, size_t bytes);
#endif
