
#include "commodore.h"

namespace testbench
{
namespace
{

#include "c64_basic_hex.inc"

}

class C64 : public commodore
{

public:

    C64()
    {
        memory.add_ROM(0xa000, c64_basic, 0x2000);
        memory.add_ROM(0xe000, c64_basic + 0x2000, c64_basic_len - 0x2000);
        memory.add_RAM(0, 0x10000);

        *mem_top_high = 0xa0;
        *mem_top_low = 0x00;
        *mem_bottom_high = 0x08;
        *mem_bottom_low = 0x00;
        *basic_entry_high = 0xe3;
        *basic_entry_low = 0x94;
        *screen_width = 40;
        *screen_height = 25;
    }

private:

    static registrar reg;

};

static machine* create_c64()
{
    return new C64;
}

machine_implementation::registrar C64::reg =
    machine_implementation::registrar("Commodore64", create_c64);

}
