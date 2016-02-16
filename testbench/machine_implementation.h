
#ifndef TESTBENCH_MACHINE_IMPLEMENTATION_MACHINE_H
#define TESTBENCH_MACHINE_IMPLEMENTATION_MACHINE_H

#include "machine.h"

#include <cstdarg>

namespace testbench
{

class machine_implementation : public machine
{
public:

    virtual ~machine_implementation();

    void enable_trace(FILE*) final;
    void disable_trace() final;
    bool is_trace_enabled() final
    {
        return trace_out != nullptr;
    }

protected:

    int charin()
    {
        return fgetc(input);
    }

    void charout(int c)
    {
        putc(c, output);
    }

    void print_trace(const char *format, ...)
    {
        if (is_trace_enabled()) {
            va_list args;
            va_start(args, format);
            vfprintf(trace_out, format, args);
            va_end(args);
            fflush(trace_out);
        }
    }

    class registrar
    {
        public:
        registrar(const char *name, machine*(*constructor)());
    };

private:

	FILE *input = nullptr;
    FILE *output = nullptr;
    FILE *monitor_in = nullptr;
    FILE *monitor_out = nullptr;
    FILE *trace_out = nullptr;

};

}
#endif
