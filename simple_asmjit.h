#ifndef SIMPLE_ASMJIT_H
#define SIMPLE_ASMJIT_H

#include "executor.h"

class SimpleAsmjit : public Executor {
public:
    SimpleAsmjit() {};
    void pre_execute_in_parsing_phase(const Program& p, bool verbose) override {};
    void execute(const Program& p, bool verbose) override;
};

#endif
