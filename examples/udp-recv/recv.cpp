#include "mxnetwork/socket.hpp"
#include <iostream>
#include <string>
#include <cstdlib>


int main(int argc, char **argv) {

    try {



    } catch(const mxnetwork::Exception &e) {
        std::cerr << "Exception: " << e.text() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
