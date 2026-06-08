#ifndef _MXSOCKET_H__
#define _MXSOCKET_H__

#include "mxnetwork/exception.hpp"
#include "mxnetwork/mxsocket.hpp"
#include <optional>
#include <string>
#include <string_view>

namespace mxnetwork {

    enum class SocketType {
        TYPE_INVALID = 0,
        TYPE_INET,
        TYPE_UNIX,
        TYPE_INET_DGRAM,
        TYPE_UNIX_DGRAM
    };

    class Socket {
      public:
        Socket(SocketType type) noexcept;
        virtual ~Socket();
        Socket(int sockfd, SocketType type);
        Socket(const MXSocket &s);
        Socket(const Socket &s) = delete;
        Socket(Socket &&s);
        Socket &operator=(const Socket &s) = delete;
        Socket &operator=(Socket &&s);
        int sockfd() const;
        bool connect(const std::string_view host, const std::string_view port);
        bool connect_unix(const std::string_view path);
        bool listen(std::string_view port, int backlog);
        bool listen_unix(std::string_view path, int backlog);
        bool setblocking(bool block);
        bool bind(std::string_view port);
        std::optional<Socket> accept();

        ssize_t read(void *buf, size_t bytes, int flags);
        bool readline(char **buffer, size_t *len);
        ssize_t write(const void *buf, size_t bytes, int flags);
        ssize_t read_all(void *buf, size_t bytes);
        ssize_t write_all(const void *buf, size_t bytes);
        ssize_t sendto(const void *buf, size_t bytes);
        ssize_t recvfrom(void *buf, size_t bytes);

        bool valid() const;
        bool is_open() const;
        void close();

      protected:
        MXSocket sock;
        SocketType type;

      private:
        void setsocket(const MXSocket &s);
    };
} // namespace mxnetwork

#endif
