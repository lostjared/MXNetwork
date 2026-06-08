#ifndef _MXSOCKET_H__
#define _MXSOCKET_H__

#include "mxnetwork/mxsocket.hpp"
#include "mxnetwork/exception.hpp"
#include<string>
#include<string_view>

namespace mxnetwork {

    enum SocketType { TYPE_INET=1, TYPE_UNIX };

    class Socket {
    public:
        Socket(SocketType type);
        virtual ~Socket();
        Socket(int sockfd, SocketType type);
        Socket(MXSocket &s);
        Socket(const Socket &s);
        Socket(Socket &&s);
        Socket& operator=(const Socket &s);
        Socket& operator=(Socket &&s);
        int sockfd() const;
        bool connect(const std::string_view host, const std::string_view port);
        bool connect_unix(const std::string_view path);
        bool listen(std::string_view port, int backlog);
        bool listen_unix(std::string_view path, int backlog);
        bool setblocking(bool block);
        bool accept(Socket &s);

        ssize_t read(void *buf, size_t bytes, int flags);
        bool readline(char **buffer, size_t *len);
        ssize_t write(const void *buf, size_t bytes, int flags);
        ssize_t read_all(void *buf, size_t bytes);
        ssize_t write_all(const void *buf, size_t bytes);

        bool valid() const;
        bool is_open() const;
        void close();

    protected:
        MXSocket sock;
        SocketType type;

    private:
        void setsocket(const MXSocket &s);
    };
}

#endif

