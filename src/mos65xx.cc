
/* vim: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */

/*
   TODO:
   6510 - need more accurate information about the ports, a die shot perhaps
          currently the 6502 is used for 6510 as well.
   6508 - seems to have the ports, and builtin 256 bytes of RAM - cool stuff

   6506 - ???
   6507 - ???
     "the 6507 has its NMI and IRQ pads permanently hooked up to the +5V supply"
     http://blog.visual6502.org/2010/09/6502-vs-6507.html

 */

#include "mos65xx.h"
#include "chips.h"

#include "nmos.h"

#include <array>
#include <cstring>
#include <cstdint>

namespace chipemu
{
namespace implementation
{

typedef uint_fast16_t node_id;

#include "mos6502.inc"

static chip_description<node_id> description_65XX =
{
    sizeof(pullups) / sizeof(pullups[0]),
    pullups,
    sizeof(transistors) / sizeof(transistors[0]),
    transistors,
    NODE::Vcc,
    NODE::Vss
};

namespace
{

static constexpr node_id address_bus_ids[16] =
    {NODE::AB15, NODE::AB14, NODE::AB13, NODE::AB12,
     NODE::AB11, NODE::AB10, NODE::AB9,  NODE::AB8,
     NODE::AB7,  NODE::AB6,  NODE::AB5,  NODE::AB4,
     NODE::AB3,  NODE::AB2,  NODE::AB1,  NODE::AB0};

static constexpr node_id data_bus_ids[8] =
    {NODE::DB7, NODE::DB6, NODE::DB5, NODE::DB4,
     NODE::DB3, NODE::DB2, NODE::DB1, NODE::DB0};

class implementation_6500 : protected nmos<node_id>,
                            protected virtual MOS6500
{
protected:

    const node_id * const pinout;

public:

    virtual void pin_write(unsigned index, bool value) noexcept
    {
        if (index > 0 and index <= pin_count()) {
            set_node(pinout[index - 1], value);
        }
    }

    virtual bool pin_read(unsigned index) const noexcept
    {
        if (index > 0 and index <= pin_count()) {
            return get_node(pinout[index - 1]);
        }
        else {
            return false;
        }
    }

    virtual unsigned char read_data_bus() const noexcept final
    {
        return static_cast<unsigned char>(read_nodes(data_bus_ids, 8));
    }

    virtual void write_data_bus(unsigned char value) noexcept final
    {
        write_nodes(data_bus_ids, 8, value);
    }

    virtual unsigned read_address_bus() const noexcept
    {
        return read_nodes(address_bus_ids + (16 - address_bus_width()),
                          address_bus_width());
    }

    virtual unsigned char A() const noexcept final
    {
        static constexpr node_id ids[8] =
            {NODE::A7, NODE::A6, NODE::A5, NODE::A4,
             NODE::A3, NODE::A2, NODE::A1, NODE::A0};

        return static_cast<unsigned char>(read_nodes(ids, 8));
    }

    virtual unsigned char X() const noexcept final
    {
        static constexpr node_id ids[8] =
            {NODE::X7, NODE::X6, NODE::X5, NODE::X4,
             NODE::X3, NODE::X2, NODE::X1, NODE::X0};

        return static_cast<unsigned char>(read_nodes(ids, 8));
    }

    virtual unsigned char Y() const noexcept final
    {
        static constexpr node_id ids[8] =
            {NODE::Y7, NODE::Y6, NODE::Y5, NODE::Y4,
             NODE::Y3, NODE::Y2, NODE::Y1, NODE::Y0};

        return static_cast<unsigned char>(read_nodes(ids, 8));
    }

    virtual unsigned char S() const noexcept final
    {
        static constexpr node_id ids[8] =
            {NODE::S7, NODE::S6, NODE::S5, NODE::S4,
             NODE::S3, NODE::S2, NODE::S1, NODE::S0};

        return static_cast<unsigned char>(read_nodes(ids, 8));
    }

    virtual unsigned char P() const noexcept final
    {
        static constexpr node_id ids[8] =
            {NODE::P7, NODE::P6, 0, NODE::P4,
             NODE::P3, NODE::P2, NODE::P1,     NODE::P0};

        return static_cast<unsigned char>(read_nodes(ids, 8));
    }

    virtual unsigned char PCH() const noexcept final
    {
        static constexpr node_id ids[8] =
            {NODE::PCH7, NODE::PCH6, NODE::PCH5, NODE::PCH4,
             NODE::PCH3, NODE::PCH2, NODE::PCH1, NODE::PCH0};

        return static_cast<unsigned char>(read_nodes(ids, 8));
    }

    virtual unsigned char PCL() const noexcept final
    {
        static constexpr node_id ids[8] =
            {NODE::PCL7, NODE::PCL6, NODE::PCL5, NODE::PCL4,
             NODE::PCL3, NODE::PCL2, NODE::PCL1, NODE::PCL0};

        return static_cast<unsigned char>(read_nodes(ids, 8));
    }

    virtual unsigned PC() const noexcept final
    {
        return (unsigned(PCH()) << 8) + unsigned(PCL());
    }

    virtual unsigned char IR() const noexcept final
    {
        static constexpr node_id ids[8] =
            {NODE::NOTIR7, NODE::NOTIR6, NODE::NOTIR5, NODE::NOTIR4,
             NODE::NOTIR3, NODE::NOTIR2, NODE::NOTIR1, NODE::NOTIR0};

        return static_cast<unsigned char>(read_nodes(ids, 8));
    }

    implementation_6500(const node_id *pinout_data):
        nmos(description_65XX),
        pinout(pinout_data)
    {
    }

    virtual ~implementation_6500() {}
};

}


/***************** 6502 *****************************************************/

static constexpr node_id pinout_6502[40] = {
    NODE::Vss, NODE::RDY, NODE::CLK1OUT, NODE::IRQ, 0, NODE::NMI,
    NODE::SYNC, NODE::Vcc, NODE::AB0, NODE::AB1, NODE::AB2, NODE::AB3,
    NODE::AB4, NODE::AB5, NODE::AB6, NODE::AB7, NODE::AB8, NODE::AB9,
    NODE::AB10, NODE::AB11, NODE::Vss, NODE::AB12, NODE::AB13, NODE::AB14,
    NODE::AB15, NODE::DB7, NODE::DB6, NODE::DB5, NODE::DB4, NODE::DB3,
    NODE::DB2, NODE::DB1, NODE::DB0, NODE::RW, 0, 0,
    NODE::CLK0IN, NODE::SO, NODE::CLK2OUT, NODE::RES
};


class implementation_6502 : public MOS6502,
                            protected implementation_6500
{

public:

    implementation_6502():
        implementation_6500(pinout_6502)
    {}

    virtual unsigned pin_count() const noexcept final
    {
        return 40;
    }

    virtual const char *name() const noexcept final
    {
        return "MOS6502";
    }

    virtual unsigned address_bus_width() const noexcept final
    {
        return 16;
    }

    virtual ~implementation_6502() {}
};

static auto reg_6502 = registrar("MOS6502", (chip*(*)())MOS6502::create);

/***************** 6502 *****************************************************/


/***************** 6503 *****************************************************/


static constexpr node_id pinout_6503[28] = {
    NODE::RES, NODE::Vss, NODE::IRQ, NODE::NMI, NODE::Vcc,
    NODE::AB0, NODE::AB1, NODE::AB2, NODE::AB3, NODE::AB4,  NODE::AB5,
    NODE::AB6, NODE::AB7, NODE::AB8, NODE::AB9, NODE::AB10, NODE::AB11,
    NODE::DB7, NODE::DB6, NODE::DB5, NODE::DB4, NODE::DB3,  NODE::DB2,
    NODE::DB1, NODE::DB0, NODE::RW,  NODE::CLK0IN, NODE::CLK2OUT
};

class implementation_6503 : public MOS6503,
                            protected implementation_6500
{
public:

    implementation_6503():
        implementation_6500(pinout_6503)
    {}

    virtual unsigned pin_count() const noexcept final
    {
        return 28;
    }

    virtual const char *name() const noexcept final
    {
        return "MOS6503";
    }

    virtual unsigned address_bus_width() const noexcept final
    {
        return 11;
    }

    virtual ~implementation_6503() {}
};

static auto reg_6503 = registrar("MOS6503", (chip*(*)())MOS6503::create);

/***************** 6503 *****************************************************/

/***************** 6504 *****************************************************/


static constexpr node_id pinout_6504[28] = {
    NODE::RES, NODE::Vss, NODE::IRQ,  NODE::Vcc, NODE::AB0,  NODE::AB1,
    NODE::AB2, NODE::AB3, NODE::AB4,  NODE::AB5, NODE::AB6,  NODE::AB7,
    NODE::AB8, NODE::AB9, NODE::AB10, NODE::AB11, NODE::AB12,
    NODE::DB7, NODE::DB6, NODE::DB5,  NODE::DB4, NODE::DB3,  NODE::DB2,
    NODE::DB1, NODE::DB0, NODE::RW,   NODE::CLK0IN, NODE::CLK2OUT
};

class implementation_6504 : public MOS6504,
                            protected implementation_6500
{
public:

    implementation_6504():
        implementation_6500(pinout_6504)
    {}

    virtual unsigned pin_count() const noexcept final
    {
        return 28;
    }

    virtual const char *name() const noexcept final
    {
        return "MOS6504";
    }

    virtual unsigned address_bus_width() const noexcept final
    {
        return 12;
    }

    virtual ~implementation_6504() {}
};

static auto reg_6504 = registrar("MOS6504", (chip*(*)())MOS6504::create);

/***************** 6504 *****************************************************/

/***************** 6505 *****************************************************/


static constexpr node_id pinout_6505[28] = {
    NODE::RES, NODE::Vss, NODE::RDY, NODE::IRQ,  NODE::Vcc, NODE::AB0,
    NODE::AB1, NODE::AB2, NODE::AB3, NODE::AB4,  NODE::AB5, NODE::AB6,
    NODE::AB7, NODE::AB8, NODE::AB9, NODE::AB10, NODE::AB11,
    NODE::DB7, NODE::DB6, NODE::DB5, NODE::DB4, NODE::DB3,  NODE::DB2,
    NODE::DB1, NODE::DB0, NODE::RW,  NODE::CLK0IN, NODE::CLK2OUT
};

class implementation_6505 : public MOS6505,
                            protected implementation_6500
{
public:

    implementation_6505():
        implementation_6500(pinout_6505)
    {}

    virtual unsigned pin_count() const noexcept final
    {
        return 28;
    }

    virtual const char *name() const noexcept final
    {
        return "MOS6505";
    }

    virtual unsigned address_bus_width() const noexcept final
    {
        return 11;
    }

    virtual ~implementation_6505() {}
};

static auto reg_6505 = registrar("MOS6505", (chip*(*)())MOS6505::create);

/***************** 6505 *****************************************************/



class implementation_6500_with_IO : public implementation_6500
{
protected:

    /*  From Commodore datasheets:
     * 
     *  Addess Enable Control (AEC) 
     * 
     *    The Address Bus, R/W and Data Bus are valid only
     *  when the Address Enable Control line is high. When low,
     *  the Address Bus, R/W and Data Bus are in a high-
     *  impedance state. This feature allows easy DMA and
     *  multiprocessor systems.
     */
    bool is_aec_high;

    /*  From Commodore datasheets:
     *
     *  I/O Port (P0-P7)
     *
     *    Eight pins are used for the peripheral port, which can
     *  transfer data to or from peripheral devices. The Output
     *  Register is located in RAM at Address 0001, and the Data
     *  Direction Register is at Addess 0000. The outputs are
     *  capable at driving one standard TTL load and 130 pf.
     */ 
    unsigned char ioports;
    unsigned char iodirs;

    void port_hoook()
    {
        unsigned address = read_address_bus();

        if (address == 0) {
            if (get_node(NODE::RW)) {
                iodirs = read_data_bus();
            }
            else {
                write_data_bus(iodirs);
            }
        }
        else if (address == 1) {
            write_data_bus(ioports);
        }
    }

public:

    enum {
        aec_node_id = 0xffef,
        ioport_id_0 = 0xfff0,
        ioport_id_1 = 0xfff1,
        ioport_id_2 = 0xfff2,
        ioport_id_3 = 0xfff3,
        ioport_id_4 = 0xfff4,
        ioport_id_5 = 0xfff5,
        ioport_id_6 = 0xfff6,
        ioport_id_7 = 0xfff7
    };

    implementation_6500_with_IO(const node_id *pinout):
        implementation_6500(pinout),
        is_aec_high(false),
        ioports(0),
        iodirs(0)
    {
    }

    virtual void pin_write(unsigned index, bool value) noexcept final
    {
        if (index > 0 and index <= pin_count()) {
            node_id id = pinout[index - 1];

            if (id == aec_node_id) {
                is_aec_high = value;
            }
            else if (id >= ioport_id_0 and id <= ioport_id_7) {
                unsigned port_index = id - ioport_id_0;

                if (not (iodirs & (1 << port_index))) {
                    if (value) {
                        ioports |= 1 << port_index;
                    }
                    else {
                        ioports &= ~(1 << port_index);
                    }
                }
            }
            else {
                set_node(id, value);
            }
        }
    }

    virtual bool pin_read(unsigned index) const noexcept final
    {
        if (index > 0 and index <= pin_count()) {
            node_id id = pinout[index - 1];

            if (id == aec_node_id)
            {
                return is_aec_high;
            }
            else if (id >= ioport_id_0 and id <= ioport_id_7) {
                unsigned port_index = id - ioport_id_0;

                if (iodirs & (1 << port_index)) {
                    return (ioports & (1 << port_index)) != 0;
                }
                else {
                    return false;
                }
            }
            else {
                return get_node(id);
            }
        }
        else {
            return false;
        }
    }

    virtual unsigned pin_count() const noexcept final
    {
        return 40;
    }

    virtual unsigned address_bus_width() const noexcept final
    {
        return 16;
    }

    virtual ~implementation_6500_with_IO() {}
};

/***************** 6510 *****************************************************/


static constexpr node_id pinout_6510[40] = {
    NODE::CLK1IN, NODE::RDY,  NODE::IRQ,  NODE::NMI,
    implementation_6500_with_IO::aec_node_id, NODE::Vcc,
    NODE::AB0,    NODE::AB1,  NODE::AB2,  NODE::AB3, NODE::AB4,  NODE::AB5,
    NODE::AB6,    NODE::AB7,  NODE::AB8, NODE::AB9,  NODE::AB10, NODE::AB11,
    NODE::AB12,   NODE::AB13, NODE::Vss, NODE::AB14, NODE::AB15,
    implementation_6500_with_IO::ioport_id_5,
    implementation_6500_with_IO::ioport_id_4,
    implementation_6500_with_IO::ioport_id_3,
    implementation_6500_with_IO::ioport_id_2,
    implementation_6500_with_IO::ioport_id_1,
    implementation_6500_with_IO::ioport_id_0,
    NODE::DB7, NODE::DB6, NODE::DB5, NODE::DB4,     NODE::DB3,  NODE::DB2,
    NODE::DB1, NODE::DB0, NODE::RW,  NODE::CLK2OUT, NODE::RES
};

class implementation_6510 : public MOS6510,
                            protected implementation_6500_with_IO
{
public:

    implementation_6510():
        implementation_6500_with_IO(pinout_6510)
    {
    }

    virtual const char *name() const noexcept final
    {
        return "MOS6510";
    }

    virtual ~implementation_6510() {}
};

static auto reg_6510 = registrar("MOS6510", (chip*(*)())MOS6510::create);

/***************** 6510 *****************************************************/

/***************** 6510_1 *****************************************************/

static constexpr node_id pinout_6510_1[40] = {
    NODE::RES,  NODE::CLK1IN,  NODE::IRQ,
    implementation_6500_with_IO::aec_node_id, NODE::Vcc,  NODE::AB0,
    NODE::AB1,  NODE::AB2, NODE::AB3,  NODE::AB4,  NODE::AB5,  NODE::AB6,
    NODE::AB7,  NODE::AB8, NODE::AB9,  NODE::AB10, NODE::AB11, NODE::AB12,
    NODE::AB13, NODE::Vss, NODE::AB14, NODE::AB15,
    implementation_6500_with_IO::ioport_id_7,
    implementation_6500_with_IO::ioport_id_6,
    implementation_6500_with_IO::ioport_id_5,
    implementation_6500_with_IO::ioport_id_4,
    implementation_6500_with_IO::ioport_id_3,
    implementation_6500_with_IO::ioport_id_2,
    implementation_6500_with_IO::ioport_id_1,
    implementation_6500_with_IO::ioport_id_0,
    NODE::DB7, NODE::DB6, NODE::DB5, NODE::DB4,    NODE::DB3,  NODE::DB2,
    NODE::DB1, NODE::DB0, NODE::RW,  NODE::CLK2IN
};


class implementation_6510_1 : public MOS6510_1,
                            protected implementation_6500_with_IO
{
public:

    implementation_6510_1():
        implementation_6500_with_IO(pinout_6510_1)
    {
    }

    virtual const char *name() const noexcept final
    {
        return "MOS6510-1";
    }

    virtual ~implementation_6510_1() {}
};

static auto reg_6510_1 = registrar("MOS6510_1", (chip*(*)())MOS6510_1::create);

/***************** 6510_1 *****************************************************/

/***************** 6510_2 *****************************************************/

static constexpr node_id pinout_6510_2[40] = {
    NODE::RES,  NODE::CLK1OUT, NODE::IRQ,
    implementation_6500_with_IO::aec_node_id, NODE::Vcc,  NODE::AB0,
    NODE::AB1,  NODE::AB2, NODE::AB3,  NODE::AB4,  NODE::AB5,  NODE::AB6,
    NODE::AB7,  NODE::AB8, NODE::AB9,  NODE::AB10, NODE::AB11, NODE::AB12,
    NODE::AB13, NODE::Vss, NODE::AB14, NODE::AB15,
    implementation_6500_with_IO::ioport_id_7,
    implementation_6500_with_IO::ioport_id_6,
    implementation_6500_with_IO::ioport_id_5,
    implementation_6500_with_IO::ioport_id_4,
    implementation_6500_with_IO::ioport_id_3,
    implementation_6500_with_IO::ioport_id_2,
    implementation_6500_with_IO::ioport_id_1,
    implementation_6500_with_IO::ioport_id_0,
    NODE::DB7, NODE::DB6, NODE::DB5, NODE::DB4,    NODE::DB3,  NODE::DB2,
    NODE::DB1, NODE::DB0, NODE::RW,  NODE::CLK2OUT
};


class implementation_6510_2 : public MOS6510_2,
                            protected implementation_6500_with_IO
{
public:

    implementation_6510_2():
        implementation_6500_with_IO(pinout_6510_2)
    {
    }

    virtual const char *name() const noexcept final
    {
        return "MOS6510-2";
    }

    virtual ~implementation_6510_2() {}
};

static auto reg_6510_2 = registrar("MOS6510_2", (chip*(*)())MOS6510_2::create);

/***************** 6510_2 *****************************************************/

}

MOS6500::~MOS6500() {}

MOS6502 *MOS6502::create()
{
    return new implementation::implementation_6502;
}

MOS6502::~MOS6502() {}

MOS6503 *MOS6503::create()
{
    return new implementation::implementation_6503;
}

MOS6503::~MOS6503() {}

MOS6504 *MOS6504::create()
{
    return new implementation::implementation_6504;
}

MOS6504::~MOS6504() {}

MOS6505 *MOS6505::create()
{
    return new implementation::implementation_6505;
}

MOS6505::~MOS6505() {}

MOS6510 *MOS6510::create()
{
    return new implementation::implementation_6510;
}

MOS6510::~MOS6510() {}

MOS6510_1 *MOS6510_1::create()
{
    return new implementation::implementation_6510_1;
}

MOS6510_1::~MOS6510_1() {}

MOS6510_2 *MOS6510_2::create()
{
    return new implementation::implementation_6510_2;
}

MOS6510_2::~MOS6510_2() {}

}
