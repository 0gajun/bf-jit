#ifndef SIMPLE_INTERP_H
#define SIMPLE_INTERP_H

#include "executor.h"
#include <vector>
#include <iostream>

class SimpleInterpreter : public Executor {
public:
    SimpleInterpreter() {};
    void pre_execute_in_parsing_phase(const Program& p, bool verbose) override {};
    void execute(const Program& p, bool verbose) override;
};

#endif
