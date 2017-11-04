#ifndef SIMPLE_JIT_H
#define SIMPLE_JIT_H

#include "executor.h"

class SimpleJit : public Executor {
public:
    SimpleJit() {};
    void pre_execute_in_parsing_phase(const Program& p, bool verbose) override {};
    void execute(const Program& p, bool verbose) override;
};

#endif
