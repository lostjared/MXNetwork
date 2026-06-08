#include "mxnetwork/socket.hpp"
#include <exception>
#include <format>
#include <iostream>
#include <string>
namespace mxnetwork {

    Socket::Socket(SocketType stype) noexcept {
        type = stype;
        if (mx_socket_init(&sock))
            return;
        else
            type = SocketType::TYPE_INVALID;
    }

    Socket::~Socket() {
        if (sock.sockfd != -1) {
            std::cout << "Socket: " << sock.sockfd << " closed.\n";
            close();
        }
    }

    Socket::Socket(int sockfd, SocketType stype) {
        if (!mx_socket_init(&sock))
            throw Exception("Error on socket init.\n");

        sock.sockfd = sockfd;
        sock.addrlen = 0;
        sock.blocking = false;
        type = stype;
    }

    Socket::Socket(const MXSocket &s) {
        setsocket(s);
    }

    Socket::Socket(Socket &&s) {
        setsocket(s.sock);
        type = s.type;
        s.sock.sockfd = -1;
    }

    Socket &Socket::operator=(Socket &&s) {
        setsocket(s.sock);
        type = s.type;
        s.sock.sockfd = -1;
        return *this;
    }

    bool Socket::connect(const std::string_view host, const std::string_view port) {
        if (type == SocketType::TYPE_INET)
            return mx_socket_connect(&sock, std::string(host).c_str(), std::string(port).c_str(), SOCK_STREAM);
        else if (type == SocketType::TYPE_INET_DGRAM)
            return mx_socket_connect(&sock, std::string(host).c_str(), std::string(port).c_str(), SOCK_DGRAM);

        return false;
    }

    bool Socket::connect_unix(const std::string_view path) {
        if (type == SocketType::TYPE_UNIX)
            return mx_socket_unix_connect(&sock, std::string(path).c_str(), SOCK_STREAM);
        else if (type == SocketType::TYPE_UNIX_DGRAM)
            return mx_socket_unix_connect(&sock, std::string(path).c_str(), SOCK_DGRAM);
        return false;
    }

    bool Socket::listen(std::string_view port, int backlog) {
        if (type == SocketType::TYPE_INET)
            return mx_socket_listen(&sock, std::string(port).c_str(), backlog, SOCK_STREAM);
        else if (type == SocketType::TYPE_INET_DGRAM)
            return mx_socket_listen(&sock, std::string(port).c_str(), backlog, SOCK_DGRAM);
        return false;
    }

    bool Socket::listen_unix(std::string_view path, int backlog) {
        if (type == SocketType::TYPE_UNIX)
            return mx_socket_unix_listen(&sock, std::string(path).c_str(), backlog, SOCK_STREAM);
        else if (type == SocketType::TYPE_UNIX_DGRAM)
            return mx_socket_unix_listen(&sock, std::string(path).c_str(), backlog, SOCK_DGRAM);
        return false;
    }

    std::optional<Socket> Socket::accept() {
        MXSocket newsocket;
        if (mx_socket_accept(&sock, &newsocket)) {
            return Socket(newsocket);
        }
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK || errno == ECONNABORTED || errno == EINVAL || errno == EBADF)
            return std::nullopt;

        throw Exception("Error accept socket failed.\n");
    }

    bool Socket::bind(std::string_view port) {
        if(!mx_socket_bind(&sock, std::string(port).c_str())) {
            throw Exception("Could not bind UDP socket.");
        }
        return true;
    }

    bool Socket::setblocking(bool block) {
        return mx_socket_set_blocking(&sock, block);
    }

    bool Socket::valid() const {
        return mx_socket_valid(&sock);
    }

    bool Socket::is_open() const {
        return mx_socket_is_open(&sock);
    }

    void Socket::close() {
        if (sock.sockfd >= 0)
            mx_socket_close(&sock);
    }

    int Socket::sockfd() const {
        return sock.sockfd;
    }

    ssize_t Socket::read(void *buf, size_t bytes, int flags) {
        return mx_socket_read(&sock, buf, bytes, flags);
    }

    bool Socket::readline(char **buffer, size_t *len) {
        return mx_socket_readline(&sock, buffer, len);
    }

    ssize_t Socket::write(const void *buf, size_t bytes, int flags) {
        return mx_socket_send(&sock, buf, bytes, flags);
    }

    ssize_t Socket::read_all(void *buf, size_t bytes) {
        return mx_socket_read_all(&sock, buf, bytes);
    }

    ssize_t Socket::write_all(const void *buf, size_t bytes) {
        return mx_socket_write_all(&sock, buf, bytes);
    }

    ssize_t Socket::sendto(const void *buf, size_t bytes) {
        if (type == SocketType::TYPE_INET_DGRAM)
            return mx_socket_sendto(&sock, buf, bytes);
        else if (type == SocketType::TYPE_UNIX_DGRAM)
            return mx_socket_unix_sendto(&sock, buf, bytes);
        return 0;
    }
    ssize_t Socket::recvfrom(void *buf, size_t bytes) {
        if (type == SocketType::TYPE_INET_DGRAM)
            return mx_socket_recvfrom(&sock, buf, bytes);
        else if (type == SocketType::TYPE_UNIX_DGRAM)
            return mx_socket_unix_recvfrom(&sock, buf, bytes);
        return 0;
    }

    void Socket::setsocket(const MXSocket &s) {
        if (!mx_socket_init(&sock))
            return;
        sock.sockfd = s.sockfd;
        sock.blocking = s.blocking;
        sock.addrlen = s.addrlen;
    }
} // namespace mxnetwork
