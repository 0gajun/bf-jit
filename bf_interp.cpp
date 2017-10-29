#include "utils.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

struct Program {
    std::string instructions;
};

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

constexpr int MEMORY_SIZE = 30000;

void simpleinterp(const Program& p, bool verbose) {
    // Initialize state
    std::vector<uint8_t> memory(MEMORY_SIZE, 0);
    size_t pc = 0;
    size_t dataptr = 0;

    while (pc < p.instructions.size()) {
        char insn = p.instructions[pc];
        switch (insn) {
            case '>':
                dataptr++;
                break;
            case '<':
                dataptr--;
                break;
            case '+':
                memory[dataptr]++;
                break;
            case '-':
                memory[dataptr]--;
                break;
            case '.':
                std::cout.put(memory[dataptr]);
                break;
            case ',':
                memory[dataptr] = std::cin.get();
                break;
            case '[':
                if (memory[dataptr] == 0) {
                    int bracket_nesting = 1;
                    size_t saved_pc = pc;

                    while (bracket_nesting && ++pc < p.instructions.size()) {
                        if (p.instructions[pc] == ']') {
                            bracket_nesting--;
                        } else if (p.instructions[pc] == '[') {
                            bracket_nesting++;
                        }
                    }

                    if (!bracket_nesting) {
                        break;
                    } else {
                        std::cerr << "Fatal: unmatched ']' at pc=" << saved_pc;
                    }
                }
                break;
            case ']':
                if (memory[dataptr] != 0) {
                    int bracket_nesting = 1;
                    size_t saved_pc = pc;

                    while (bracket_nesting && pc > 0) {
                        pc--;
                        if (p.instructions[pc] == '[') {
                            bracket_nesting--;
                        } else if (p.instructions[pc] == ']') {
                            bracket_nesting++;
                        }
                    }

                    if (!bracket_nesting) {
                        break;
                    } else {
                        std::cerr << "Fatal: unmatched '[' at pc=" << saved_pc;
                    }
                }
                break;
            default:
                std::cerr << "Fatal: bad char'" << insn << "'at pc=" << pc;
                break;
        }
        pc++;
    }
}

int main(int argc, const char** argv) {
    bool verbose = false;
    std::string bf_file_path;

    parse_command_line(argc, argv, &bf_file_path, &verbose);

    std::ifstream file(bf_file_path);
    if (!file) {
        std::cerr << "Fatal: Unable to open file " << bf_file_path << std::endl;
        exit(1);
    }
    Timer t1;
    Program program = parse_from_stream(file);

    if (verbose) {
        std::cout << "Parsing took: " << t1.elapsed() << "s\n";
        std::cout << "Length of program: " << program.instructions.size() << "\n";
        std::cout << "Program: \n" <<  program.instructions << "\n";
    }

    if (verbose) {
        std::cout << "[>] Running simpleinterp: \n";
    }

    Timer t2;
    simpleinterp(program, verbose);

    if (verbose) {
        std::cout << "\n[<] Done (elapsed: " << t2.elapsed() << "s)\n";
    }

    return 0;
}
