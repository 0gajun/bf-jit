#include "opt1_interp.h"

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
