#include "opt2_interp.h"
#include <stack>

#ifdef BFTRACE
#include <unordered_map>
#endif

Opt2Interpreter::Opt2Interpreter() {}

size_t calculate_repeated_insn_count(const Program& p, size_t pc) {
    char insn = p.instructions[pc];
    size_t c = 0;
    while (insn == p.instructions[pc + c]) {
        c++;
    }
    return c;
}

std::vector<BfOp> parse_bf_ops(const Program& p) {
    std::vector<BfOp> ops;

    size_t pc = 0;

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
                ops.push_back(BfOp(BfOpKind::JUMP_IF_DATA_ZERO, 0));
                pc += 1;
                break;
            case ']':
                ops.push_back(BfOp(BfOpKind::JUMP_IF_DATA_NOT_ZERO, 0));
                pc += 1;
                break;
            default:
                std::cerr << "Fatal: bad char'" << insn << "'at pc=" << pc;
                break;
        }
    }

    pc = 0;
    std::stack<size_t> block_map;

    while (pc < ops.size()) {
        //std::cout << "pc: " << pc << ",\t kind=" << get_kind_str(ops[pc].kind) << ", \t arg=" << ops[pc].argument << std::endl;
        if (ops[pc].kind == BfOpKind::JUMP_IF_DATA_ZERO) {
            block_map.push(pc);
        }
        if (ops[pc].kind == BfOpKind::JUMP_IF_DATA_NOT_ZERO) {
            size_t target_pc = block_map.top();
            size_t offset = pc - target_pc;
            ops[pc].argument = offset;
            ops[target_pc].argument = offset;
            block_map.pop();
        }
        pc++;
    }
    return ops;
}

void Opt2Interpreter::pre_execute_in_parsing_phase(const Program& p, bool verbose) {
    this->bf_ops = parse_bf_ops(p);
}

void Opt2Interpreter::execute(const Program& p, bool verbose) {
    // Initialize state
    std::vector<uint8_t> memory(MEMORY_SIZE, 0);
    size_t pc = 0;
    size_t dataptr = 0;

#ifdef BFTRACE
    std::unordered_map<std::string, size_t> op_exec_count;
#endif

    while (pc < bf_ops.size()) {
        const BfOp& op = bf_ops[pc];

#ifdef BFTRACE
        op_exec_count[get_kind_str(op.kind)]++;
#endif

        switch (op.kind) {
            case BfOpKind::INC_PTR:
                dataptr += op.argument;
                break;
            case BfOpKind::DEC_PTR:
                dataptr -= op.argument;
                break;
            case BfOpKind::INC_DATA:
                memory[dataptr] += op.argument;
                break;
            case BfOpKind::DEC_DATA:
                memory[dataptr] -= op.argument;
                break;
            case BfOpKind::WRITE_STDOUT:
                if (op.argument != 1) {
                    std::cerr << "Fatal: write stdout op, arg=" << op.argument << std::endl;
                    exit(1);
                }
                std::cout.put(memory[dataptr]);
                break;
            case BfOpKind::READ_STDIN:
                if (op.argument != 1) {
                    std::cerr << "Fatal: read stdin op" << op.argument << std::endl;
                    exit(1);
                }
                memory[dataptr] = std::cin.get();
                break;
            case BfOpKind::JUMP_IF_DATA_ZERO:
                if (memory[dataptr] == 0) {
                    pc += op.argument;
                }
                break;
            case BfOpKind::JUMP_IF_DATA_NOT_ZERO:
                if (memory[dataptr] != 0) {
                    pc -= op.argument;
                }
                break;
            default:
                std::cerr << "Fatal: Unknown op at pc=" << pc;
                break;
        }
        pc++;
    }

#ifdef BFTRACE
    std::cout << "* Tracing:\n";
    std::cout.imbue(std::locale(""));
    size_t total = 0;
    for (auto i : op_exec_count) {
        std::cout << i.first << "\t--> " << i.second << std::endl;
        total += i.second;
    }
    std::cout << ".. Total: " << total << "\n";
#endif
}

std::string get_kind_str(BfOpKind kind) {
    switch (kind) {
        case BfOpKind::INC_PTR:
            return "INC_PTR";
        case BfOpKind::DEC_PTR:
            return "DEC_PTR";
        case BfOpKind::INC_DATA:
            return "INC_DATA";
        case BfOpKind::DEC_DATA:
            return "DEC_DATA";
        case BfOpKind::WRITE_STDOUT:
            return "WRITE_STDOUT";
        case BfOpKind::READ_STDIN:
            return "READ_STDIN";
        case BfOpKind::JUMP_IF_DATA_ZERO:
            return "JUMP_IF_DATA_ZERO";
        case BfOpKind::JUMP_IF_DATA_NOT_ZERO:
            return "JUMP_IF_DATA_NOT_ZERO";
        default:
            return "UNKNOWN";
    }
}
