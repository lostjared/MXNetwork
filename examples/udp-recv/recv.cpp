#include "mxnetwork/socket.hpp"
#include <atomic>
#include <cstdlib>
#include <iostream>
#include <signal.h>
#include <string>
#include <vector>

std::atomic<bool> active_loop(false);

void exit_signal(int) {
    active_loop.store(false);
}

int main(int argc, char **argv) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return EXIT_FAILURE;
    }
    struct sigaction sa;
    sa.sa_handler = exit_signal;
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    try {
        mxnetwork::Socket sock(mxnetwork::SocketType::TYPE_INET_DGRAM);
        if (sock.bind(argv[1])) {
            std::cout << "Listening for UDP datagram on port " << argv[1] << "...\n";
            std::vector<uint8_t> buffer(65536);
            active_loop.store(true);
            while (active_loop.load()) {
                ssize_t bytes = sock.recvfrom(buffer.data(), buffer.size());
                if (bytes > 0) {
                    std::cout << "Received " << bytes << " bytes.\n";
                } else if (bytes < 0) {
                    if (errno == EINTR)
                        continue;
                    std::cerr << "read error.\n";
                    break;
                }
            }
        }
    } catch (const mxnetwork::Exception &e) {
        std::cerr << "Exception: " << e.text() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
