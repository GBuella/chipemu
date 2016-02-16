
#include "commodore.h"

namespace testbench
{
namespace
{

#include "cvic20_basic_hex.inc"

}

class CVIC20 : public commodore
{

public:

    CVIC20()
    {
        memory.add_ROM(0xc000, cvic20_basic, cvic20_basic_len);
        memory.add_RAM(0, 0x1400);

        *mem_top_high = 0x14;
        *mem_top_low = 0x00;
        *mem_bottom_high = 0x06;
        *mem_bottom_low = 0x00;
        *basic_entry_high = cvic20_basic[1];
        *basic_entry_low = cvic20_basic[0];
        *screen_width = 22;
        *screen_height = 23;
    }

private:

    static registrar reg;

};

static machine* create_vic20()
{
    return new CVIC20;
}

machine_implementation::registrar CVIC20::reg =
    machine_implementation::registrar("CommodoreVIC20", create_vic20);

}
