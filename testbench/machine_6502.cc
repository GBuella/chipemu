
#include "machine_6502.h"

#include "mos65xx.h"

using chipemu::MOS6502;

testbench::machine_6502::machine_6502():
    CPU_6502(MOS6502::create())
{}

static void initialize(MOS6502*);
static void cycle(MOS6502*);
static void handle_memory(MOS6502*, testbench::memory*);

inline void testbench::machine_6502::trace_CPU()
{
    if (not is_trace_enabled()) return;

    print_trace("A:%02X X:%02X Y:%02X P:%02X PC:%04X S:%02X"
                " IR:%02X RW:%d AB:%04X DB:%02X\n",
                CPU()->A(), CPU()->X(), CPU()->Y(), CPU()->P(),
                CPU()->PC(), CPU()->S(),
                (~(CPU()->IR())) & 0xff,
                CPU()->pin_read(MOS6502::RW),
                CPU()->read_address_bus(),
                CPU()->read_data_bus());
    if (CPU()->pin_read(MOS6502::RW)) {
        print_trace(" read $%04X - $%02X\n",
                    CPU()->read_address_bus(),
                    memory.read(CPU()->read_address_bus()));
    }
    else {
        print_trace(" write $%04X - $%02X\n",
                    CPU()->read_address_bus(),
                    CPU()->read_data_bus());
    }
}

inline void testbench::machine_6502::initialize_CPU()
{
    print_trace("Initializing MOS6502\n");
    CPU_6502->pin_write(MOS6502::RES, false);              //  setup pins for
    CPU_6502->recalc();                                    //   starting the CPU_6502
    CPU_6502->pin_write(MOS6502::CLK0IN, true);            //   the order is important,
    CPU_6502->recalc();                                    //   
    CPU_6502->pin_write(MOS6502::RDY, true);
    CPU_6502->recalc();
    CPU_6502->pin_write(MOS6502::SO, false);
    CPU_6502->recalc();
    CPU_6502->pin_write(MOS6502::IRQ, true);
    CPU_6502->recalc();
    CPU_6502->pin_write(MOS6502::NMI, true);

    CPU_6502->stabilize_network();                         // compute initial states
                                                      //  of all nodes

    print_trace("Initializing MOS6502 - holding RES\n");
    for (int clk = 1; clk <= 8; ++clk) {
        cycle(CPU_6502.get());                        // hold reset for 8 cycles
        //handle_memory(CPU_6502.get(), &memory);
        trace_CPU();
    }

    CPU_6502->pin_write(MOS6502::RES, true);               // than let it go
    CPU_6502->recalc();

    print_trace("Initializing MOS6502 - done\n");
}

void testbench::machine_6502::run(FILE *input, FILE *output)
{
    std::lock_guard<std::mutex> lock(mutex);
    unsigned long long cycle_count = 0;

    initialize_CPU();
    while (not feof(input)) {
        ++cycle_count;
        cycle(CPU_6502.get());
        handle_memory(CPU_6502.get(), &memory);
        print_trace("Cycle %llu\n", cycle_count);
        trace_CPU();
        on_CPU_cycle(input, output, cycle_count);
    }
}

testbench::machine_6502::~machine_6502()
{
}

static void handle_memory(MOS6502 *CPU, testbench::memory *memory)
{
    unsigned address = CPU->read_address_bus();

    if (CPU->pin_read(MOS6502::RW)) {
        CPU->write_data_bus(memory->read(address));
    }
    else {
        memory->write(address, CPU->read_data_bus());
    }
}

static void cycle(MOS6502 *CPU)
{
    CPU->pin_write(MOS6502::CLK0IN, false);   //  do the two halfcycles
    CPU->recalc();                            //   todo: trace between
    CPU->pin_write(MOS6502::CLK0IN, true);    //   halfcycles
    CPU->recalc();
}

