#include "mxnetwork/socket.hpp"
#include <atomic>
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <signal.h>
#include <thread>

std::atomic<bool> active_loop{false};

void exit_signal(int) {
    active_loop.store(false);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "server: Invalid arguments\n"
                  << argv[0] << " <port>\n";
        return EXIT_FAILURE;
    }

    struct sigaction sa;
    sa.sa_handler = exit_signal;
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, nullptr) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    try {
        mxnetwork::Socket sock(mxnetwork::SocketType::TYPE_INET);
        if (sock.listen(argv[1], 5)) {
            active_loop.store(true);
            while (active_loop.load()) {
                std::optional<mxnetwork::Socket> s = sock.accept();
                if (s) {
                    std::thread t([&sock](mxnetwork::Socket sfd) {
                        char buffer[256];
                        ssize_t bytes = 0;
                        if ((bytes = sfd.read_all(buffer, 255)) > 0) {
                            buffer[bytes] = '\0';
                            std::string value{buffer};
                            if (value.find("exit") != std::string::npos) {
                                active_loop.store(false);
                                std::cerr << "server: " << ": Exiting..\n";
                                shutdown(sock.sockfd(), SHUT_RDWR);
                                return;
                            }
                            std::cout << value << "\n";
                        } else {
                            std::cerr << "Error reading stream.\n";
                        }
                    },
                                  std::move(*s));
                    t.detach();
                } else {
                    if (errno == EINTR)
                        continue;
                    break;
                }
            }
        } else {
            perror("listen");
        }
    } catch (const mxnetwork::Exception &s) {
        std::cerr << "Exception: " << s.text() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
