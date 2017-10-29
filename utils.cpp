#include "utils.h"
#include <iostream>
#include <cstdlib>

Timer::Timer() :t1_(std::chrono::high_resolution_clock::now()) {};

double Timer::elapsed() {
    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = t2 - t1_;
    return elapsed.count();
}

void parse_command_line(int argc, const char** argv, std::string* bf_file_path, bool* verbose) {
    *verbose = false;

    int arg_i = 1;
    for (; arg_i < argc; ++arg_i) {
        std::string arg = argv[arg_i];
        if (!(arg.size() > 2 && arg[0] == '-' && arg[1] == '-')) {
            // If this arg doesn't start with a --, it's not a flag.
            break;
        } else if (arg == "--verbose"){
            *verbose = true;
        } else {
            exit(1);
        }
    }

    if (arg_i >= argc) {
        std::cout << "You must specify bf_file_path" << std::endl;
        exit(1);
    }
    *bf_file_path = argv[arg_i];
}
