# MXNetwork

MXNetwork is a small POSIX C++20 socket library. It provides a low-level C API around an `MXSocket` struct and a move-only C++ RAII wrapper, `mxnetwork::Socket`, for TCP, UDP, Unix stream, and Unix datagram sockets.

The project builds a static library named `mxnetwork` and includes several example programs for common socket workflows.

## Features

- TCP client and server sockets over IPv4.
- UDP datagram sockets over IPv4.
- Unix domain stream and datagram sockets.
- Blocking and non-blocking socket mode support.
- Move-only C++ socket ownership with automatic close in the destructor.
- Helpers for `read`, `write`, `read_all`, `write_all`, `sendto`, `recvfrom`, and line reads.
- CMake install/export support for `find_package(mxnetwork)`.

## Requirements

- CMake 3.10 or newer.
- A C++20 compiler.
- POSIX socket headers and APIs, including `sys/socket.h`, `sys/un.h`, `arpa/inet.h`, `fcntl.h`, `poll.h`, and `unistd.h`.
- pthreads, discovered through CMake `Threads`.
- Qt6 Core, Widgets, and Network only if building the optional relay client example.

## Build

```sh
cmake -S . -B build
cmake --build build
```

By default, CMake builds the static library and the command-line examples:

- `download-file`
- `mxnetwork-client`
- `mxnetwork-server`
- `mxnetwork-recv-udp`
- `mxnetwork-send-udp`
- `mxnetwork-unix-recv-udp`
- `mxnetwork-unix-send-udp`
- `mxnetwork-relay`

Debug builds enable stricter warnings for GCC and address sanitizer unless the optional Qt client is enabled:

```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

Release mode is used when no debug build type is selected.

## Optional Qt Relay Client

The Qt relay client is disabled by default. Enable it with `CLIENT=ON`:

```sh
cmake -S . -B build-client -DCLIENT=ON
cmake --build build-client
```

This adds the `relay_client` executable from `examples/relay-client`.

## Install

```sh
cmake --install build
```

The install exports a CMake package. A consuming project can link the installed library like this:

```cmake
find_package(mxnetwork REQUIRED)

add_executable(my-app main.cpp)
target_link_libraries(my-app PRIVATE mxnetwork::mxnetwork)
```

When using this repository directly as a subdirectory, link against the local alias:

```cmake
add_subdirectory(path/to/MXNetwork)

add_executable(my-app main.cpp)
target_link_libraries(my-app PRIVATE libmxnetwork::mxnetwork)
```

## Basic Usage

```cpp
#include "mxnetwork/socket.hpp"

#include <iostream>
#include <string>

int main() {
    mx_socket_ignore_pipe_signal();

    mxnetwork::Socket sock(mxnetwork::SocketType::TYPE_INET);
    if (!sock.connect("127.0.0.1", "8080")) {
        std::cerr << "connect failed\n";
        return 1;
    }

    std::string message = "hello";
    if (sock.write_all(message.data(), message.size()) < 0) {
        std::cerr << "write failed\n";
        return 1;
    }

    return 0;
}
```

The main socket types are:

- `mxnetwork::SocketType::TYPE_INET` for IPv4 TCP sockets.
- `mxnetwork::SocketType::TYPE_UNIX` for Unix stream sockets.
- `mxnetwork::SocketType::TYPE_INET_DGRAM` for IPv4 UDP sockets.
- `mxnetwork::SocketType::TYPE_UNIX_DGRAM` for Unix datagram sockets.

## Examples

After building, run examples from the build directory.

TCP server and client:

```sh
./build/examples/server/mxnetwork-server 9000
./build/examples/client/mxnetwork-client 127.0.0.1 9000
```

UDP receiver and sender:

```sh
./build/examples/udp-recv/mxnetwork-recv-udp 9001
./build/examples/udp-send/mxnetwork-send-udp 127.0.0.1 9001
```

Unix datagram receiver and sender:

```sh
./build/examples/unix-udp-recv/mxnetwork-unix-recv-udp /tmp/mxnetwork.sock
./build/examples/unix-udp-send/mxnetwork-unix-send-udp /tmp/mxnetwork.sock
```

Relay server:

```sh
./build/examples/relay/mxnetwork-relay 9002
```

HTTP file download example:

```sh
./build/examples/download/download-file example.com 80 /index.html index.html
```

## API Overview

Include `mxnetwork/socket.hpp` to use the C++ wrapper. The wrapper owns the file descriptor, closes it on destruction, cannot be copied, and can be moved.

Common `mxnetwork::Socket` methods:

- `connect(host, port)` and `connect_unix(path)`.
- `listen(port, backlog)` and `listen_unix(path, backlog)`.
- `accept()` returning `std::optional<mxnetwork::Socket>`.
- `bind(port)` and `bind_unix(path)` for datagram receivers.
- `setblocking(bool)`.
- `read`, `write`, `read_all`, `write_all`, and `readline`.
- `sendto` and `recvfrom` for datagram sockets.
- `valid()`, `is_open()`, `sockfd()`, `socket_type()`, and `close()`.

Include `mxnetwork/mxsocket.hpp` to use the C-style API directly.

## License

MXNetwork is licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE) for the full license text.
