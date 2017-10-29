#ifndef OPT1_INTERP_H
#define OPT1_INTERP_H

#include "executor.h"
#include <vector>
#include <iostream>

//#define BFTRACE

class Opt1Interpreter : public Executor {
public:
    Opt1Interpreter();
    void pre_execute_in_parsing_phase(const Program& p, bool verbose) override;
    void execute(const Program& p, bool verbose) override;

private:
    std::vector<size_t> jumptable;
    void compute_jumptable(const Program& p);
};

#endif
