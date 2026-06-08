#include "mxnetwork/mxsocket.hpp"
#include <signal.h>
#include <unistd.h>

[[nodiscard]] bool mx_socket_unix_listen(MXSocket *sock, const char *path, int backlog, int type) {
    if (path == nullptr)
        return false;
    if (!mx_socket_init(sock))
        return false;
    struct sockaddr_un addr;
    int sockfd = socket(AF_UNIX, type, 0);
    if (sockfd == -1) {
        perror("socket");
        return false;
    }
    if (remove(path) == -1 && errno == ENOENT) {
        perror("remove");
        close(sockfd);
        return false;
    }
    memset(&addr, 0, sizeof(struct sockaddr_un));
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    addr.sun_family = AF_UNIX;
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        close(sockfd);
        return false;
    }

    if (listen(sockfd, backlog) == -1) {
        perror("listen");
        close(sockfd);
        return false;
    }
    sock->sockfd = sockfd;
    sock->sun = addr;
    return true;
}

[[nodiscard]] bool mx_socket_unix_connect(MXSocket *sock, const char *path, int type) {
    if (path == nullptr)
        return false;
    if (!mx_socket_init(sock))
        return false;
    int sockfd = -1;
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    sockfd = socket(AF_UNIX, type, 0);
    if (sockfd == -1) {
        perror("socket");
        return false;
    }
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        close(sockfd);
        return false;
    }
    sock->sockfd = sockfd;
    sock->sun = addr;
    return true;
}

[[nodiscard]] bool mx_socket_listen(MXSocket *sock, const char *port, int backlog, int type) {
    if (port == nullptr)
        return false;

    if (!mx_socket_init(sock))
        return false;

    struct addrinfo hints;
    struct addrinfo *rt, *rp;
    int sfd = -1, optval, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;
    hints.ai_socktype = type;
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_PASSIVE;
    s = getaddrinfo(nullptr, port, &hints, &rt);
    if (s != 0)
        return false;

    optval = 1;

    for (rp = rt; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
            close(sfd);
            freeaddrinfo(rt);
            return false;
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        if (sfd >= 0)
            close(sfd);
        sfd = -1;
    }

    if (sfd == -1) {
        freeaddrinfo(rt);
        return false;
    }

    if (rp != nullptr && sfd >= 0) {
        if (listen(sfd, backlog) == -1) {
            freeaddrinfo(rt);
            close(sfd);
            return false;
        }
        sock->sockfd = sfd;
        sock->addrlen = rp->ai_addrlen;
        memcpy(&sock->inet, rp->ai_addr, rp->ai_addrlen);
    } else {
        if (sfd >= 0)
            close(sfd);
        return false;
    }

    freeaddrinfo(rt);
    return true;
}

[[nodiscard]] bool mx_socket_accept(const MXSocket *input, MXSocket *output) {
    if (input == nullptr || output == nullptr)
        return false;

    if (!mx_socket_valid(input))
        return false;

    int newfd = accept(input->sockfd, 0, 0);
    if (newfd == -1)
        return false;

    int flags = fcntl(newfd, F_GETFL);
    if (flags == -1) {
        close(newfd);
        return false;
    }
    if (input->blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    if (fcntl(newfd, F_SETFL, flags) == -1) {
        close(newfd);
        return false;
    }
    if (mx_socket_valid(output))
        mx_socket_close(output);
    output->sockfd = newfd;
    output->addrlen = input->addrlen;
    output->blocking = input->blocking;
    output->sun = input->sun;
    output->inet = input->inet;
    return true;
}

void mx_socket_close(MXSocket *sock) {
    if (sock == nullptr)
        return;
    if (sock->sockfd >= 0)
        close(sock->sockfd);
    sock->sockfd = -1;
}

[[nodiscard]] bool mx_socket_set_blocking(MXSocket *sock, bool state) {
    if (sock == nullptr)
        return false;
    if (sock->sockfd >= 0) {
        int flags = fcntl(sock->sockfd, F_GETFL);
        if (flags == -1) {
            fprintf(stderr, "Error getting flags for: %d\n", sock->sockfd);
            return false;
        }
        if (state)
            flags &= ~O_NONBLOCK;
        else
            flags |= O_NONBLOCK;
        if (fcntl(sock->sockfd, F_SETFL, flags) == -1) {
            fprintf(stderr, "Error setting flags for: %d\n", sock->sockfd);
            return false;
        }
        sock->blocking = state;
    } else
        return false;
    return true;
}

[[nodiscard]] bool mx_socket_connect(MXSocket *sock, const char *host, const char *port, int type) {
    if (host == nullptr || port == nullptr)
        return false;

    if (!mx_socket_init(sock))
        return false;

    struct addrinfo hints;
    struct addrinfo *rt, *rp;
    int sfd = -1, s;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = type;
    s = getaddrinfo(host, port, &hints, &rt);
    if (s != 0) {
        errno = ENOSYS;
        return false;
    }
    for (rp = rt; rp != nullptr; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;

        if (sfd >= 0) {
            close(sfd);
            sfd = -1;
        }
    }

    if (rp != nullptr) {
        sock->sockfd = sfd;
        sock->addrlen = rp->ai_addrlen;
        memcpy(&sock->inet, rp->ai_addr, rp->ai_addrlen);
    } else {
        freeaddrinfo(rt);
        if (sfd >= 0)
            close(sfd);
        return false;
    }

    freeaddrinfo(rt);
    return true;
}

[[nodiscard]] bool mx_socket_bind(MXSocket *sock, const char *port) {
    if(!mx_socket_init(sock))
        return false;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct addrinfo hints = {0};
    struct addrinfo *result = nullptr;
    struct addrinfo *rp = nullptr;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    int s = getaddrinfo(nullptr, port, &hints, &result);
    if (s != 0) {
        return false;
    }
    bool set_value = false;
    for(rp = result; rp != nullptr; rp = rp->ai_next) {
        if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            memcpy(&sock->inet, rp->ai_addr, rp->ai_addrlen);
            sock->addrlen = rp->ai_addrlen;
            set_value = true;
            break;
        }
    }
    freeaddrinfo(result);
    if(!set_value)
        return false;
    sock->sockfd = sockfd;
    return true;
}

[[nodiscard]] bool mx_socket_init(MXSocket *sock) {
    if (sock == nullptr)
        return false;
    memset(sock, 0, sizeof(MXSocket));
    sock->sockfd = -1;
    sock->blocking = true;
    return true;
}

[[nodiscard]] bool mx_socket_valid(const MXSocket *sock) {
    if (sock == nullptr)
        return false;
    return sock->sockfd >= 0;
}

ssize_t mx_socket_read(MXSocket *sock, void *buf, size_t len, int flags) {
    if (sock == nullptr || buf == nullptr || len == 0)
        return -1;
    if (!mx_socket_valid(sock)) {
        errno = EBADF;
        return -1;
    }
    return recv(sock->sockfd, buf, len, flags);
}

ssize_t mx_socket_send(MXSocket *sock, const void *buf, size_t len, int flags) {
    if (sock == nullptr || buf == nullptr || len == 0)
        return -1;
    if (!mx_socket_valid(sock)) {
        errno = EBADF;
        return -1;
    }
    return send(sock->sockfd, buf, len, flags);
}

[[nodiscard]] bool mx_socket_is_open(const MXSocket *sock) {
    if (sock == nullptr || !mx_socket_valid(sock))
        return false;
    char c = 0;
    ssize_t r = recv(sock->sockfd, &c, 1, MSG_PEEK | MSG_DONTWAIT);
    if (r == 0)
        return false;
    if (r > 0)
        return true;
    if (r < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return true;
    }
    return false;
}

[[nodiscard]] bool mx_socket_readline(MXSocket *sock, char **buffer, size_t *size) {
    if (sock == nullptr || buffer == nullptr || size == nullptr)
        return false;

    if (!mx_socket_valid(sock)) {
        errno = EBADF;
        return false;
    }

    *buffer = nullptr;
    *size = 0;

    size_t init_size = 4096;
    char *temp = (char *)malloc(init_size + 1);
    if (temp == nullptr)
        return false;
    char c = 0;
    size_t index = 0;
    while (1) {
        ssize_t read_val = recv(sock->sockfd, &c, 1, 0);
        if (read_val > 0) {
            if (c == '\n')
                break;
            if (index >= init_size) {
                size_t new_init_size = init_size * 2;
                char *t = (char *)realloc(temp, new_init_size + 1);
                if (t == nullptr) {
                    free(temp);
                    return false;
                }
                temp = t;
                init_size = new_init_size;
            }
            temp[index++] = c;
            continue;
        }
        if (read_val == 0) {
            if (index == 0) {
                free(temp);
                return false;
            }
            break;
        }
        if (errno == EINTR)
            continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (!sock->blocking)
                break;
            continue;
        }
        free(temp);
        return false;
    }
    temp[index] = 0;
    *buffer = temp;
    *size = index;
    return true;
}

ssize_t mx_socket_write_all(MXSocket *sock, const void *buf, size_t bytes) {
    const char *ptr = (const char *)buf;
    size_t left = bytes;
    while (left > 0) {
        ssize_t written = write(sock->sockfd, ptr, left);
        if (written == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        left -= (size_t)written;
        ptr += written;
    }
    return (ssize_t)bytes;
}

ssize_t mx_socket_read_all(MXSocket *sock, void *buf, size_t bytes) {
    char *ptr = (char *)buf;
    size_t left = bytes;
    while (left > 0) {
        ssize_t bytes_read = read(sock->sockfd, ptr, left);
        if (bytes_read == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (bytes_read == 0)
            break;
        left -= (size_t)bytes_read;
        ptr += bytes_read;
    }
    return (ssize_t)(bytes - left);
}

void mx_socket_ignore_pipe_signal() {
    signal(SIGPIPE, SIG_IGN);
}

ssize_t mx_socket_sendto(MXSocket *sock, const void *buf, size_t src_bytes) {
    ssize_t bytes = 0;
    bytes = sendto(sock->sockfd, buf, src_bytes, 0, (struct sockaddr *)&sock->inet, sock->addrlen);
    return bytes;
}
ssize_t mx_socket_recvfrom(MXSocket *sock, void *buf, size_t src_bytes) {
    ssize_t bytes = 0;
    socklen_t len = sizeof(struct sockaddr_storage);
    struct sockaddr_storage caddr;
    bytes = recvfrom(sock->sockfd, buf, src_bytes, 0, (struct sockaddr *)&caddr, &len);
    return bytes;
}

ssize_t mx_socket_unix_sendto(MXSocket *sock, const void *buf, size_t src_bytes) {
    ssize_t bytes = 0;
    bytes = sendto(sock->sockfd, buf, src_bytes, 0, (struct sockaddr *)&sock->sun, sock->addrlen);
    return bytes;
}
ssize_t mx_socket_unix_recvfrom(MXSocket *sock, void *buf, size_t src_bytes) {
    ssize_t bytes = 0;
    socklen_t len = sizeof(struct sockaddr_storage);
    struct sockaddr_storage caddr;
    bytes = recvfrom(sock->sockfd, buf, src_bytes, 0, (struct sockaddr *)&caddr, &len);
    return bytes;
}
