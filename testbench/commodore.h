
#ifndef TESTBENCH_COMMODORE_H
#define TESTBENCH_COMMODORE_H

/* A base class for all 6502 based commodore microcomputers
   -- for now, this includes 6510 as well

   Assume these all come with a kernal ROM, and a basic ROM
 */

#include "machine_6502.h"

namespace testbench
{

class commodore : public machine_6502
{
protected:
    commodore();
    virtual ~commodore();

    virtual void on_CPU_cycle(FILE *input, FILE *output,
                              unsigned long long cycle) override final;

    const std::shared_ptr<address_range> kernel_registers;

    unsigned char * const mem_top_high;
    unsigned char * const mem_top_low;
    unsigned char * const mem_bottom_high;
    unsigned char * const mem_bottom_low;
    unsigned char * const basic_entry_high;
    unsigned char * const basic_entry_low;
    unsigned char * const screen_width;
    unsigned char * const screen_height;
};

}

#endif
