#include "simple_interp.h"

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

