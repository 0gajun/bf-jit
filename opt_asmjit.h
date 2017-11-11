#ifndef OPT_ASMJIT_H
#define OPT_ASMJIT_H

#include "executor.h"

#include <vector>

enum BfOpKind {
    INVALID_OP = 0,
    INC_PTR,
    DEC_PTR,
    INC_DATA,
    DEC_DATA,
    READ_STDIN,
    WRITE_STDOUT,
    LOOP_SET_TO_ZERO,
    LOOP_MOVE_PTR,
    LOOP_MOVE_DATA,
    JUMP_IF_DATA_ZERO,
    JUMP_IF_DATA_NOT_ZERO,
};

struct BfOp {
    BfOp(BfOpKind kind, int64_t argument) : kind(kind), argument(argument) {};

    BfOpKind kind = BfOpKind::INVALID_OP;
    int64_t argument = 0;
};

class OptAsmjit : public Executor {
public:
    OptAsmjit() {};
    void pre_execute_in_parsing_phase(const Program& p, bool verbose) override;
    void execute(const Program& p, bool verbose) override;

private:
    std::vector<BfOp> bf_ops;
};

#endif
