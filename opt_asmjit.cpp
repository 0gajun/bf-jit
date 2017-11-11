#include "opt_asmjit.h"
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

size_t calculate_repeated_insn_count(const Program& p, size_t pc) {
    char insn = p.instructions[pc];
    size_t c = 0;
    while (insn == p.instructions[pc + c]) {
        c++;
    }
    return c;
}

std::vector<BfOp> optimize_loop(const std::vector<BfOp>& ops, size_t loop_start) {
    std::vector<BfOp> new_ops;

    if (ops.size() - loop_start == 2) {
        BfOp repeated_op = ops[loop_start + 1];
        switch (repeated_op.kind) {
        case BfOpKind::INC_DATA:
        case BfOpKind::DEC_DATA:
            new_ops.push_back(BfOp(BfOpKind::LOOP_SET_TO_ZERO, 0));
            break;
        case BfOpKind::INC_PTR:
            new_ops.push_back(BfOp(BfOpKind::LOOP_MOVE_PTR, repeated_op.argument));
            break;
        case BfOpKind::DEC_PTR:
            new_ops.push_back(BfOp(BfOpKind::LOOP_MOVE_PTR, -repeated_op.argument));
            break;
        default:
            break;
        }
    } else if (ops.size() - loop_start == 5) {
        if (ops[loop_start + 1].kind == BfOpKind::DEC_DATA
                && ops[loop_start + 3].kind == BfOpKind::INC_DATA
                && ops[loop_start + 1].argument == 1
                && ops[loop_start + 3].argument == 1) {
            if (ops[loop_start + 2].kind == BfOpKind::INC_PTR
                    && ops[loop_start + 4].kind == BfOpKind::DEC_PTR
                    && ops[loop_start + 2].argument == ops[loop_start + 4].argument) {
                new_ops.push_back(BfOp(BfOpKind::LOOP_MOVE_DATA, ops[loop_start + 2].argument));
            } else if (ops[loop_start + 2].kind == BfOpKind::DEC_PTR
                    && ops[loop_start + 4].kind == BfOpKind::INC_PTR
                    && ops[loop_start + 2].argument == ops[loop_start + 4].argument) {
                new_ops.push_back(BfOp(BfOpKind::LOOP_MOVE_DATA, -ops[loop_start + 2].argument));
            }
        }
    }
    return new_ops;
}

std::vector<BfOp> parse_bf_ops(const Program& p) {
    std::vector<BfOp> ops;

    size_t pc = 0;

    std::stack<size_t> loop_block_stack;

    while (pc < p.instructions.size()) {
        size_t repeated_count = calculate_repeated_insn_count(p, pc);
        char insn = p.instructions[pc];
        switch (insn) {
            case '>':
                ops.push_back(BfOp(BfOpKind::INC_PTR, repeated_count));
                pc += repeated_count;
                break;
            case '<':
                ops.push_back(BfOp(BfOpKind::DEC_PTR, repeated_count));
                pc += repeated_count;
                break;
            case '+':
                ops.push_back(BfOp(BfOpKind::INC_DATA, repeated_count));
                pc += repeated_count;
                break;
            case '-':
                ops.push_back(BfOp(BfOpKind::DEC_DATA, repeated_count));
                pc += repeated_count;
                break;
            case '.':
                ops.push_back(BfOp(BfOpKind::WRITE_STDOUT, repeated_count));
                pc += repeated_count;
                break;
            case ',':
                ops.push_back(BfOp(BfOpKind::READ_STDIN, repeated_count));
                pc += repeated_count;
                break;
            case '[':
                loop_block_stack.push(ops.size());
                ops.push_back(BfOp(BfOpKind::JUMP_IF_DATA_ZERO, 0));
                pc++;
                break;
            case ']':
                {
                    size_t loop_start = loop_block_stack.top();
                    loop_block_stack.pop();

                    std::vector<BfOp> optimized_loop = optimize_loop(ops, loop_start);

                    if (optimized_loop.empty()) {
                        ops[loop_start].argument = ops.size();
                        ops.push_back(BfOp(BfOpKind::JUMP_IF_DATA_NOT_ZERO, loop_start));
                    } else {
                        ops.erase(ops.begin() + loop_start, ops.end());
                        ops.insert(ops.end(), optimized_loop.begin(), optimized_loop.end());
                    }
                    pc++;
                }
                break;
            default:
                std::cerr << "Fatal: bad char'" << insn << "'at pc=" << pc;
                break;
        }
    }

    return ops;
}

void OptAsmjit::pre_execute_in_parsing_phase(const Program& p, bool verbose) {
    this->bf_ops = parse_bf_ops(p);
}

void OptAsmjit::execute(const Program& p, bool verbose) {
    asmjit::JitRuntime rt;
    asmjit::CodeHolder code;
    code.init(rt.getCodeInfo());
    asmjit::X86Assembler assm(&code);

    asmjit::X86Gp dataptr = asmjit::x86::r13;
    assm.mov(dataptr, asmjit::x86::rdi);

    std::stack<BracketLabels> open_bracket_stack;

    for (size_t pc = 0; pc < bf_ops.size(); pc++) {
        BfOp op = bf_ops[pc];
        switch (op.kind) {
            case BfOpKind::INC_PTR:
                assm.add(dataptr, op.argument);
                break;
            case BfOpKind::DEC_PTR:
                assm.sub(dataptr, op.argument);
                break;
            case BfOpKind::INC_DATA:
                assm.add(asmjit::x86::byte_ptr(dataptr), op.argument);
                break;
            case BfOpKind::DEC_DATA:
                assm.sub(asmjit::x86::byte_ptr(dataptr), op.argument);
                break;
            case BfOpKind::READ_STDIN:
                for (size_t i = 0; i < op.argument; i++) {
                    assm.call(asmjit::imm_ptr(mygetchar));
                    assm.mov(asmjit::x86::byte_ptr(dataptr), asmjit::x86::al);
                }
                break;
            case BfOpKind::WRITE_STDOUT:
                for (size_t i = 0; i < op.argument; i++) {
                    assm.movzx(asmjit::x86::rdi, asmjit::x86::byte_ptr(dataptr));
                    assm.call(asmjit::imm_ptr(myputchar));
                }
                break;
            case BfOpKind::LOOP_SET_TO_ZERO:
                assm.mov(asmjit::x86::byte_ptr(dataptr), 0);
                break;
            case BfOpKind::LOOP_MOVE_PTR:
                {
                    asmjit::Label begin_label = assm.newLabel();
                    asmjit::Label end_label = assm.newLabel();
                    assm.bind(begin_label);
                    assm.cmp(asmjit::x86::byte_ptr(dataptr), 0);

                    assm.jz(end_label);
                    if (op.argument < 0) {
                        assm.sub(dataptr, -op.argument);
                    } else {
                        assm.add(dataptr, op.argument);
                    }
                    assm.jmp(begin_label);
                    assm.bind(end_label);
                }
                break;
            case BfOpKind::LOOP_MOVE_DATA:
                {
                    asmjit::Label skip_move = assm.newLabel();
                    assm.cmp(asmjit::x86::byte_ptr(dataptr), 0);
                    assm.jz(skip_move);

                    assm.mov(asmjit::x86::r14, dataptr);
                    if (op.argument < 0) {
                      assm.sub(asmjit::x86::r14, -op.argument);
                    } else {
                      assm.add(asmjit::x86::r14, op.argument);
                    }
                    assm.mov(asmjit::x86::rax, asmjit::x86::byte_ptr(dataptr));
                    assm.add(asmjit::x86::byte_ptr(asmjit::x86::r14), asmjit::x86::al);
                    assm.mov(asmjit::x86::byte_ptr(dataptr), 0);
                    assm.bind(skip_move);
                }
                break;
            case BfOpKind::JUMP_IF_DATA_ZERO:
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
            case BfOpKind::JUMP_IF_DATA_NOT_ZERO:
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
            case BfOpKind::INVALID_OP:
            default:
                std::cerr << "Fatal: Unknown op at pc=" << pc << "(" << op.kind << ")";
                exit(1);
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

    std::vector<uint8_t> memory(MEMORY_SIZE, 0);
    func((uint64_t) memory.data());

    std::cout << "successfully finished" << std::endl;

    rt.release(func);
}
