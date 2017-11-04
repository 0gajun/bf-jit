#include "simple_jit.h"
#include "jit_utils.h"

#include <stack>
#include <iostream>

void SimpleJit::execute(const Program& p, bool verbose) {
    std::vector<uint8_t> memory(MEMORY_SIZE, 0);

    CodeEmitter emitter;

    std::stack<size_t> loop_block_stack;

    // movabs <address of memory.data>, %r13
    emitter.EmitBytes({0x49, 0xBD});
    emitter.EmitUint64((uint64_t)memory.data());

    for (size_t pc = 0; pc < p.instructions.size(); pc++) {
        char insn = p.instructions[pc];
        switch (insn) {
            case '>':
                // inc %r13
                emitter.EmitBytes({0x49, 0xFF, 0xC5});
                break;
            case '<':
                // dec %r13
                emitter.EmitBytes({0x49, 0xFF, 0xCD});
                break;
            case '+':
                // addb $1, 0(%r13)
                emitter.EmitBytes({0x41, 0x80, 0x45, 0x00, 0x01});
                break;
            case '-':
                // subb $1, 0(%r13)
                emitter.EmitBytes({0x41, 0x80, 0x6D, 0x00, 0x01});
                break;
            case '.':
                // To emit one byte to stdout, call the write syscall with fd=1 (for
                // stdout), buf=address of byte, count=1.
                //
                // mov $1, %rax
                // mov $1, %rdi
                // mov %r13, %rsi
                // mov $1, %rdx
                // syscall
                emitter.EmitBytes({0x48, 0xC7, 0xC0, 0x01, 0x00, 0x00, 0x00});
                emitter.EmitBytes({0x48, 0xC7, 0xC7, 0x01, 0x00, 0x00, 0x00});
                emitter.EmitBytes({0x4C, 0x89, 0xEE});
                emitter.EmitBytes({0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00});
                emitter.EmitBytes({0x0F, 0x05});
                break;
            case ',':
                // To read one byte from stdin, call the read syscall with fd=0 (for
                // stdin),
                // buf=address of byte, count=1.
                emitter.EmitBytes({0x48, 0xC7, 0xC0, 0x00, 0x00, 0x00, 0x00});
                emitter.EmitBytes({0x48, 0xC7, 0xC7, 0x00, 0x00, 0x00, 0x00});
                emitter.EmitBytes({0x4C, 0x89, 0xEE});
                emitter.EmitBytes({0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00});
                emitter.EmitBytes({0x0F, 0x05});
                break;
            case '[':
                // cmpb $0, 0(%r13)
                emitter.EmitBytes({0x41, 0x80, 0x7d, 0x00, 0x00});
                loop_block_stack.push(emitter.size());
                // jz <place holder 0>
                emitter.EmitBytes({0x0F, 0x84});
                emitter.EmitUint32(0);
                break;
            case ']':
                {
                    if (loop_block_stack.empty()) {
                        std::cerr << "Unmatched closing ']' at pc=" << pc;
                        exit(1);
                    }
                    size_t loop_start = loop_block_stack.top();
                    loop_block_stack.pop();

                    // cmpb $0, 0(%r13)
                    emitter.EmitBytes({0x41, 0x80, 0x7d, 0x00, 0x00});
                    size_t jump_back_from = emitter.size() + 6;
                    size_t jump_back_to = loop_start + 6;
                    uint32_t pcrel_offset_back = compute_relative_32bit_offset(jump_back_from, jump_back_to);

                    // jnz <loop_start location>
                    emitter.EmitBytes({0x0F, 0x85});
                    emitter.EmitUint32(pcrel_offset_back);

                    size_t jump_forward_from = loop_start + 6;
                    size_t jump_forward_to = emitter.size();
                    uint32_t pcrel_offset_forward = compute_relative_32bit_offset(jump_forward_from, jump_forward_to);
                    emitter.ReplaceUint32AtOffset(loop_start + 2, pcrel_offset_forward);
                }
                break;
            default:
                break;
        }
    }

    emitter.EmitByte(0xC3);

    std::vector<uint8_t> emitted_code = emitter.code();
    JitProgram jit_program(emitted_code);

    using JittedFunc = void (*)(void);
    JittedFunc func = (JittedFunc)jit_program.program_memory();
    func();
}

