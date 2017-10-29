#include "utils.h"
#include "opt1_interp.h"
#include "simple_interp.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

Program parse_from_stream(std::istream& stream) {
    Program program;

    for (std::string line; std::getline(stream, line);) {
        for (auto c : line) {
            if (c == '>' || c == '<' || c == '+' || c == '-' || c == '.' ||
                c == ',' || c == '[' || c == ']') {
                program.instructions.push_back(c);
            }
        }
    }

    return program;
}

std::unique_ptr<Executor> newExecutor() {
    std::unique_ptr<Executor> executor(new Opt1Interpreter());
    return executor;
}

int main(int argc, const char** argv) {
    bool verbose = false;
    std::string bf_file_path;

    parse_command_line(argc, argv, &bf_file_path, &verbose);

    std::unique_ptr<Executor> executor = newExecutor();

    std::ifstream file(bf_file_path);
    if (!file) {
        std::cerr << "Fatal: Unable to open file " << bf_file_path << std::endl;
        exit(1);
    }
    Timer t1;
    Program program = parse_from_stream(file);

    executor->pre_execute_in_parsing_phase(program, verbose);

    if (verbose) {
        std::cout << "Parsing took: " << t1.elapsed() << "s\n";
        std::cout << "Length of program: " << program.instructions.size() << "\n";
        std::cout << "Program: \n" <<  program.instructions << "\n";
    }

    if (verbose) {
        std::cout << "[>] Running simpleinterp: \n";
    }

    Timer t2;
    executor->execute(program, verbose);

    if (verbose) {
        std::cout << "\n[<] Done (elapsed: " << t2.elapsed() << "s)\n";
    }

    return 0;
}
