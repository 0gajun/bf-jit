#include "simple_asmjit.h"
#include "jit_utils.h"
#include "asmjit/asmjit.h"

#include <stack>
#include <iostream>

void myputchar(uint8_t c) {
    putchar(c);
}

uint8_t mygetchar() {
    return getchar();
}

class BracketLabels {
public:
    BracketLabels(asmjit::Label open_label, asmjit::Label close_label)
        : open_label(open_label), close_label(close_label) {};

    const asmjit::Label open_label;
    const asmjit::Label close_label;
};

void SimpleAsmjit::execute(const Program& p, bool verbose) {
    std::vector<uint8_t> memory(MEMORY_SIZE, 0);

    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    code.init(rt.getCodeInfo());
    asmjit::X86Assembler assm(&code);

    asmjit::X86Gp dataptr = asmjit::x86::r13;
    assm.mov(dataptr, asmjit::x86::rdi);

    std::stack<BracketLabels> open_bracket_stack;

    for (size_t pc = 0; pc < p.instructions.size(); pc++) {
        char insn = p.instructions[pc];
        switch (insn) {
            case '>':
                // inc %r13
                assm.inc(dataptr);
                break;
            case '<':
                // dec %r13
                assm.dec(dataptr);
                break;
            case '+':
                // addb $1, 0(%r13)
                assm.add(asmjit::x86::byte_ptr(dataptr), 1);
                break;
            case '-':
                // subb $1, 0(%r13)
                assm.sub(asmjit::x86::byte_ptr(dataptr), 1);
                break;
            case '.':
                // call myputchar [dataptr]
                assm.movzx(asmjit::x86::rdi, asmjit::x86::byte_ptr(dataptr));
                assm.call(asmjit::imm_ptr(myputchar));
                break;
            case ',':
                // [dataptr] = call mygetchar
                assm.call(asmjit::imm_ptr(mygetchar));
                assm.mov(asmjit::x86::byte_ptr(dataptr), asmjit::x86::al);
                break;
            case '[':
                {
                    // cmpb $0, 0(%r13)
                    assm.cmp(asmjit::x86::byte_ptr(dataptr), 0);
                    asmjit::Label open_label = assm.newLabel();
                    asmjit::Label close_label = assm.newLabel();

                    assm.jz(close_label);

                    assm.bind(open_label);
                    open_bracket_stack.push(BracketLabels(open_label, close_label));
                }
                break;
            case ']':
                if (open_bracket_stack.empty()) {
                    std::cerr << "Unmatched closing ']' at pc=" << pc;
                    exit(1);
                }
                {
                    BracketLabels labels = open_bracket_stack.top();
                    open_bracket_stack.pop();

                    assm.cmp(asmjit::x86::byte_ptr(dataptr), 0);
                    assm.jnz(labels.open_label);
                    assm.bind(labels.close_label);
                }
                break;
            default:
                break;
        }
    }

    assm.ret();

    if (assm.isInErrorState()) {
        std::cerr << "asmjit error: " << asmjit::DebugUtils::errorAsString(assm.getLastError()) << "\n";
        exit(1);
    }

    using JittedFunc = void (*)(uint64_t);

    JittedFunc func;
    asmjit::Error err = rt.add(&func, &code);
    if (err) {
        std::cerr << "Cannot emmit asm instructions\n";
        exit(1);
    }
    func((uint64_t) memory.data());

    std::cout << "successfully finished" << std::endl;
}

