
#ifndef TESTBENCH_MACHINE_H
#define TESTBENCH_MACHINE_H

#include <cstdio>

namespace testbench
{

class machine
{
public:

    virtual void run(FILE *input, FILE *output) = 0;

    virtual void enable_trace(FILE*) = 0;
    virtual void disable_trace() = 0;
    virtual bool is_trace_enabled() = 0;

    static machine* create(const char*);

    virtual ~machine();

private:

    struct availability
    {
        static const char** begin();
        static const char** end();
        static size_t count();
        static bool is(const char *name);
    };

public:
    static struct availability available;

};

}

#endif
