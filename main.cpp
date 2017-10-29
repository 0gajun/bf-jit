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

class Executor {
public:
    virtual void pre_execute_in_parsing_phase(const Program& p, bool verbose) = 0;
    virtual void execute(const Program& p, bool verbose) = 0;
};

class SimpleInterpreter : public Executor {
public:
    SimpleInterpreter();
    void pre_execute_in_parsing_phase(const Program& p, bool verbose) override {};
    void execute(const Program& p, bool verbose) override;
};

class Opt1Interpreter : public Executor{
public:
    Opt1Interpreter();
    void pre_execute_in_parsing_phase(const Program& p, bool verbose) override;
    void execute(const Program& p, bool verbose) override;

private:
    std::vector<size_t> jumptable;
    void compute_jumptable(const Program& p);
};

Opt1Interpreter::Opt1Interpreter() {}

void Opt1Interpreter::compute_jumptable(const Program& p) {
    size_t pc = 0;
    size_t program_size = p.instructions.size();
    std::vector<size_t> jumptable(program_size, 0);

    while (pc < program_size) {
        char instruction = p.instructions[pc];
        if (instruction == '[') {
            int bracket_nesting = 1;
            size_t seek = pc;

            while (bracket_nesting && ++seek < program_size) {
                if (p.instructions[seek] == ']') {
                    bracket_nesting--;
                } else if (p.instructions[seek] == '['){
                    bracket_nesting++;
                }
            }

            if (!bracket_nesting) {
                jumptable[pc] = seek;
                jumptable[seek] = pc;
            } else {
                std::cerr << "Fatal: Unmatched '[' at pc=" << pc << std::endl;
                exit(1);
            }
        }
        pc++;
    }

    this->jumptable = jumptable;
}

void Opt1Interpreter::pre_execute_in_parsing_phase(const Program& p, bool verbose) {
    compute_jumptable(p);
}

void Opt1Interpreter::execute(const Program& p, bool verbose) {
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
                   pc = jumptable[pc];
                }
                break;
            case ']':
                if (memory[dataptr] != 0) {
                    pc = jumptable[pc];
                }
                break;
            default:
                std::cerr << "Fatal: bad char'" << insn << "'at pc=" << pc;
                break;
        }
        pc++;
    }
}

void SimpleInterpreter::execute(const Program& p, bool verbose) {
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
    //simpleinterp(program, verbose);
    executor->execute(program, verbose);

    if (verbose) {
        std::cout << "\n[<] Done (elapsed: " << t2.elapsed() << "s)\n";
    }

    return 0;
}
