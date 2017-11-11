#include "utils.h"
#include "executor.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#ifdef SIMPLE
#include "simple_interp.h"
#elif defined OPT1
#include "opt1_interp.h"
#elif defined OPT2
#include "opt2_interp.h"
#elif defined OPT3
#include "opt3_interp.h"
#elif defined SIMPLE_JIT
#include "simple_jit.h"
#elif defined SIMPLE_ASMJIT
#include "simple_asmjit.h"
#elif defined OPT_ASMJIT
#include "opt_asmjit.h"
#endif

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

Executor* __newExecutorImpl() {
#ifdef SIMPLE
    return new SimpleInterpreter();
#elif defined OPT1
    return new Opt1Interpreter();
#elif defined OPT2
    return new Opt2Interpreter();
#elif defined OPT3
    return new Opt3Interpreter();
#elif defined SIMPLE_JIT
    return new SimpleJit();
#elif defined SIMPLE_ASMJIT
    return new SimpleAsmjit();
#elif defined OPT_ASMJIT
    return new OptAsmjit();
#else
    std::cerr << "Cannot Infrate Executor Impl. Don't you forget set correct variable? (e.g. -DSIMPLE)\n";
    abort();
#endif
}

std::unique_ptr<Executor> newExecutor() {
    std::unique_ptr<Executor> executor(__newExecutorImpl());
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
