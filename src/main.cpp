#include "app.h"

#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char** argv) {
    try {
        viewer::App app;
        if (argc > 1) {
            app.setStartupModelPath(argv[1]);
        }
        app.run();
    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
