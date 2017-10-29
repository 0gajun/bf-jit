#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <string>

constexpr int MEMORY_SIZE = 30000;

struct Program {
    std::string instructions;
};

class Executor {
public:
    virtual void pre_execute_in_parsing_phase(const Program& p, bool verbose) = 0;
    virtual void execute(const Program& p, bool verbose) = 0;
};

#endif
