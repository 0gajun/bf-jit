#include "opt3_interp.h"
#include <stack>
#include <iomanip>

#ifdef BFTRACE
#include <unordered_map>
#endif

Opt3Interpreter::Opt3Interpreter() {}

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
    std::stack<size_t> block_stack;

    while (pc < ops.size()) {
        //std::cout << "pc: " << pc << ",\t kind=" << get_kind_str(ops[pc].kind) << ", \t arg=" << ops[pc].argument << std::endl;
        if (ops[pc].kind == BfOpKind::JUMP_IF_DATA_ZERO) {
            block_stack.push(pc);
        }
        if (ops[pc].kind == BfOpKind::JUMP_IF_DATA_NOT_ZERO) {
            size_t target_pc = block_stack.top();
            block_stack.pop();
            size_t offset = pc - target_pc;
            ops[pc].argument = offset;
            ops[target_pc].argument = offset;
        }
        pc++;
    }
    return ops;
}

void Opt3Interpreter::pre_execute_in_parsing_phase(const Program& p, bool verbose) {
    this->bf_ops = parse_bf_ops(p);
}

void Opt3Interpreter::execute(const Program& p, bool verbose) {
    // Initialize state
    std::vector<uint8_t> memory(MEMORY_SIZE, 0);
    size_t pc = 0;
    size_t dataptr = 0;

#ifdef BFTRACE
    std::unordered_map<int, size_t> op_exec_count;
    std::string current_trace;
    std::unordered_map<std::string, size_t> trace_count;
#endif

    while (pc < bf_ops.size()) {
        const BfOp& op = bf_ops[pc];

#ifdef BFTRACE
        op_exec_count[static_cast<int>(op.kind)]++;
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
                for (size_t i = 0; i < op.argument; i++) {
                    std::cout.put(memory[dataptr]);
                }
                break;
            case BfOpKind::READ_STDIN:
                for (size_t i = 0; i < op.argument; i++) {
                    memory[dataptr] = std::cin.get();
                }
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

#ifdef BFTRACE
        if (op.kind == BfOpKind::JUMP_IF_DATA_ZERO) {
            current_trace = "";
        } else if (op.kind == BfOpKind::JUMP_IF_DATA_NOT_ZERO) {
            trace_count[current_trace]++;
            current_trace = "";
        } else {
            current_trace += get_kind_char(op.kind) + std::to_string(op.argument);
        }
#endif
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

    std::pair<std::string, size_t> trace_pair;
    std::vector<std::pair<std::string, size_t>> trace_pairs;
    std::copy(trace_count.begin(), trace_count.end(),
            std::back_inserter<std::vector<std::pair<std::string, size_t>>>(trace_pairs));
    std::sort(trace_pairs.begin(), trace_pairs.end(),
            [](const std::pair<std::string, size_t>& a, const std::pair<std::string, size_t>& b) {
                return a.second > b.second;
            });

    for (auto const& p : trace_pairs) {
        std::cout << std::setw(15) << std::left << p.first << " --> " << p.second << "\n";
    }
#endif
}

std::string get_kind_char(BfOpKind kind) {
    switch (kind) {
        case BfOpKind::INC_PTR:
            return ">";
        case BfOpKind::DEC_PTR:
            return "<";
        case BfOpKind::INC_DATA:
            return "+";
        case BfOpKind::DEC_DATA:
            return "-";
        case BfOpKind::WRITE_STDOUT:
            return ".";
        case BfOpKind::READ_STDIN:
            return ",";
        case BfOpKind::JUMP_IF_DATA_ZERO:
            return "[";
        case BfOpKind::JUMP_IF_DATA_NOT_ZERO:
            return "]";
        default:
            return "?";
    }
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
