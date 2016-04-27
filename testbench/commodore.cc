
#include "commodore.h"
#include "mos65xx.h"

#include <cstring>
#include <array>

namespace testbench
{

namespace
{

#include "cbm_tiny_kernel.h"
#include "cbm_tiny_kernel_hex.inc"

class kernel_registers_class : public address_range
{
public:
    std::array<unsigned char, REGISTERS_SIZE> data;

    bool is_writable() const final
    {
        return true;
    }

    bool is_visible() const
    {
        return true;
    }

    bool contains(unsigned address) const final
    {
        return address >= REGISTERS_START
               and address < REGISTERS_START + REGISTERS_SIZE;
    }

    unsigned char read(unsigned address) const final
    {
        return data[address - REGISTERS_START];
    }

    void write(unsigned address, unsigned char value) final
    {
        data[address - REGISTERS_START] = value;
    }

    kernel_registers_class()
    {
        memset(data.data(), 0, data.size());
    }
};

unsigned char* register_addr(const std::shared_ptr<address_range>& registers,
                             unsigned offset)
{
    return &((static_cast<kernel_registers_class*>(registers.get()))->data[offset]);

}

}

commodore::commodore():
    kernel_registers(new kernel_registers_class),
    mem_top_high(register_addr(kernel_registers, OFF_RAM_TOP + 1)),
    mem_top_low(register_addr(kernel_registers, OFF_RAM_TOP + 0)),
    mem_bottom_high(register_addr(kernel_registers, OFF_RAM_BOTTOM + 1)),
    mem_bottom_low(register_addr(kernel_registers, OFF_RAM_BOTTOM + 0)),
    basic_entry_high(register_addr(kernel_registers, OFF_BASIC_ENTRY + 1)),
    basic_entry_low(register_addr(kernel_registers, OFF_BASIC_ENTRY + 0)),
    screen_width(register_addr(kernel_registers, OFF_SCREEN_WIDTH)),
    screen_height(register_addr(kernel_registers, OFF_SCREEN_HEIGHT))
{
    memory.add_ROM(0x10000 - cbm_tiny_kernel_len,
				   cbm_tiny_kernel,
				   cbm_tiny_kernel_len);
    memory.add_range(kernel_registers);
}

static bool handle_chrin(unsigned char *io,
                         unsigned char *ack,
                         FILE *input)
{
    if (*io == 0) {
        int c = fgetc(input);

        if (c != EOF) {
            *io = (unsigned char)c;
        }
        return false;
    }
    else {
        return *ack != 0;
    }
}

static void write_char(unsigned char c, FILE *output)
{
    fputc(c, output);
    fflush(output);
}

static bool handle_chrout(unsigned char *io, FILE *output)
{
    if (*io != 0) {
        write_char(*io, output);
        return true;
    }
    else {
        return false;
    }
}

static bool is_syscall_address(unsigned address)
{
    return (address >= 0xff81)
           and (address <= 0xfff3)
           and (address - 0xff81) % 3 == 0;
}

void commodore::on_CPU_cycle(FILE *input, FILE *output,
                             unsigned long long cycle) 
{
    unsigned char * const select = register_addr(kernel_registers, OFF_SELECT);
    unsigned char * const io = register_addr(kernel_registers, OFF_IO);
    unsigned char * const ack = register_addr(kernel_registers, OFF_ACK);
    bool clear_all;

    if (is_trace_enabled() and is_syscall_address(CPU()->PC())) {
        print_trace("syscall: $%04X\n", CPU()->PC());
    }
    switch (*select) {
        case SELECT_CHRIN:
            clear_all = handle_chrin(io, ack, input);
            break;
        case SELECT_CHROUT:
            clear_all = handle_chrout(io, output);
            break;
        case SELECT_NOT_IMPL:
            print_trace("syscall not implement\n");
            clear_all = true;
            break;
        default:
            clear_all = false;
            break;
    }
    if (clear_all) {
        *select = 0;
        *io = 0;
        *ack = 0;
    }
}

commodore::~commodore()
{
}

}
