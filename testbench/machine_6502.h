
#ifndef BASIC_MACHINE_6502_H
#define BASIC_MACHINE_6502_H

#include "machine_implementation.h"
#include "memory.h"

namespace chipemu { class MOS6502; }

#include <memory>
#include <mutex>

namespace testbench
{

class machine_6502 : public machine_implementation
{
protected:

    class memory memory;

    virtual void on_CPU_cycle(FILE *input, FILE *output,
                              unsigned long long cycle) = 0;

    machine_6502();

public:

    const chipemu::MOS6502* CPU() const
    {
        return CPU_6502.get();
    }

    ~machine_6502();

    virtual void run(FILE *input, FILE *output) override final;

private:

    void initialize_CPU();
    void trace_CPU();
    std::mutex mutex;
    const std::unique_ptr<chipemu::MOS6502> CPU_6502;

};

}

#endif
