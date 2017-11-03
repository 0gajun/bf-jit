#ifndef OPT3_INTERP_H
#define OPT3_INTERP_H

#include "executor.h"
#include <vector>
#include <iostream>

#define BFTRACE

enum class BfOpKind {
    INVALID_OP = 0,
    INC_PTR,
    DEC_PTR,
    INC_DATA,
    DEC_DATA,
    READ_STDIN,
    WRITE_STDOUT,
    JUMP_IF_DATA_ZERO,
    JUMP_IF_DATA_NOT_ZERO,
};

std::string get_kind_str(BfOpKind kind);
std::string get_kind_char(BfOpKind kind);

struct BfOp {
    BfOp(BfOpKind kind, size_t argument_param) : kind(kind), argument(argument_param) {};

    BfOpKind kind = BfOpKind::INVALID_OP;
    size_t argument = 0;
};

class Opt3Interpreter : public Executor {
public:
    Opt3Interpreter();
    void pre_execute_in_parsing_phase(const Program& p, bool verbose) override;
    void execute(const Program& p, bool verbose) override;

private:
    std::vector<BfOp> bf_ops;
    std::vector<size_t> jumptable;
    void compute_jumptable(const Program& p);
};

#endif
