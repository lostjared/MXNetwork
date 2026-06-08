#include "mxnetwork/socket.hpp"
#include<exception>
#include<iostream>
#include<string>

namespace mxnetwork {

    Socket::Socket(SocketType stype) {
        type = stype;
        if(!mx_socket_init(&sock))
            throw Exception("Error on socket init.\n");

    }

    Socket::~Socket() {

    }

    Socket::Socket(int sockfd, SocketType stype) {
        if(!mx_socket_init(&sock))
            throw Exception("Error on socket init.\n");

        sock.sockfd = sockfd;
        sock.addrlen = 0;
        sock.blocking = false;
        type = stype;
    }

    Socket::Socket(MXSocket &s) {
        setsocket(s);
    }

    Socket::Socket(const Socket &s) {
        setsocket(s.sock);
        type = s.type;
    }

    Socket::Socket(Socket &&s) {
        setsocket(s.sock);
        type = s.type;
    }

    Socket& Socket::operator=(const Socket &s) {
        setsocket(s.sock);
        type = s.type;
        return *this;
    }

    Socket& Socket::operator=(Socket &&s) {
        setsocket(s.sock);
        type = s.type;
        return *this;
    }

    bool Socket::connect(const std::string_view host, const std::string_view port) {
        if(type == SocketType::TYPE_INET)
            return mx_socket_connect(&sock, std::string(host).c_str(), std::string(port).c_str(), SOCK_STREAM);
        return false;
    }

    bool Socket::connect_unix(const std::string_view path) {
        if(type == SocketType::TYPE_INET)
            return mx_socket_unix_connect(&sock, std::string(path).c_str());
        return false;
    }

    bool Socket::listen(std::string_view port, int backlog) {
        if(type == SocketType::TYPE_INET)
            return mx_socket_listen(&sock, std::string(port).c_str(), backlog);
        return false;
    }

    bool Socket::listen_unix(std::string_view path, int backlog) {
        if(type == SocketType::TYPE_UNIX)
            return mx_socket_unix_listen(&sock,std::string(path).c_str(), backlog);
        return false;
    }

    Socket Socket::accept() {
        MXSocket newsocket;
        if(mx_socket_accept(&sock, &newsocket))
            return Socket(newsocket);
        throw Exception("Error accept socket failed.\n");
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

    void Socket::setsocket(const MXSocket &s) {
        if(!mx_socket_init(&sock))
            return;
        sock.sockfd = s.sockfd;
        sock.blocking = s.blocking;
        sock.addrlen = s.addrlen;
    }
}
