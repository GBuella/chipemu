/*
 Copyright (c) 2010,2014 Michael Steil, Brian Silverman, Barry Silverman

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "mos65xx.h"
#include <memory>

struct state_t
{
  std::unique_ptr<chipemu::MOS6502> chip;

  state_t():
	chip(chipemu::MOS6502::create())
  {}
};

#include "perfect6502.h"
#include <cstdint>

// #include "types.h"

/************************************************************
 *
 * 6502-specific Interfacing
 *
 ************************************************************/

uint16_t
readAddressBus(state_t *state)
{
  return state->chip->read_address_bus();
}

uint8_t
readDataBus(state_t *state)
{
  return state->chip->read_data_bus();
}

void
writeDataBus(state_t *state, uint8_t d)
{
  return state->chip->write_data_bus(d);
}

int
readRW(state_t *state)
{
  return state->chip->pin_read(chipemu::MOS6502::RW);
}

uint8_t
readA(state_t *state)
{
  return state->chip->A();
}

uint8_t
readX(state_t *state)
{
  return state->chip->X();
}

uint8_t
readY(state_t *state)
{
  return state->chip->Y();
}

uint8_t
readP(state_t *state)
{
  return state->chip->P();
}

uint8_t
readSP(state_t *state)
{
  return state->chip->S();
}

uint8_t
readPCL(state_t *state)
{
  return state->chip->PCL();
}

uint8_t
readPCH(state_t *state)
{
  return state->chip->PCH();
}

uint16_t
readPC(state_t *state)
{
	return (readPCH(state) << 8) | readPCL(state);
}

/************************************************************
 *
 * Address Bus and Data Bus Interface
 *
 ************************************************************/

uint8_t memory[65536];

static uint8_t
mRead(uint16_t a)
{
	return memory[a];
}

static void
mWrite(uint16_t a, uint8_t d)
{
	memory[a] = d;
}

static inline void
handleMemory(state_t *state)
{
  if (state->chip->pin_read(chipemu::MOS6502::RW)) {
	writeDataBus(state, mRead(readAddressBus(state)));
  }
  else {
	mWrite(readAddressBus(state), readDataBus(state));
  }
}

/************************************************************
 *
 * Main Clock Loop
 *
 ************************************************************/

unsigned int cycle;

void
step(state_t *state)
{
	bool clk = state->chip->pin_read(chipemu::MOS6502::CLK0IN);

	/* invert clock */
	state->chip->pin_write(chipemu::MOS6502::CLK0IN, not clk);
	state->chip->recalc();

	/* handle memory reads and writes */
	if (not clk) {
		handleMemory(state);
	}

	cycle++;
}

state_t*
initAndResetChip()
{
    using namespace chipemu;

    state_t *state = new state_t;

    chipStatus(state);
	state->chip->pin_write(MOS6502::RES, false);
	state->chip->recalc();
	state->chip->pin_write(MOS6502::CLK0IN, true);
	state->chip->recalc();
	state->chip->pin_write(MOS6502::RDY, true);
	state->chip->recalc();
	state->chip->pin_write(MOS6502::SO, false);
	state->chip->recalc();
	state->chip->pin_write(MOS6502::IRQ, true);
	state->chip->recalc();
	state->chip->pin_write(MOS6502::NMI, true);

	state->chip->stabilize_network();

	/* hold RESET for 8 cycles */
	for (int i = 0; i < 16; i++) {
    chipStatus(state);
		step(state);
  }

	state->chip->pin_write(MOS6502::RES, true);
	state->chip->recalc();

	cycle = 0;

	return state;
}

void delete_chip(state_t *state)
{
  delete state;
}

/************************************************************
 *
 * Tracing/Debugging
 *
 ************************************************************/

void
chipStatus(state_t *state)
{
  return;
	bool clk = state->chip->pin_read(chipemu::MOS6502::CLK0IN);
	uint16_t a = state->chip->read_address_bus();
	uint8_t d = state->chip->read_data_bus();
	bool r_w = state->chip->pin_read(chipemu::MOS6502::RW);
  int ir = 0xff & ~state->chip->IR();

	printf("halfcyc:%d phi0:%d AB:%04X D:%02X RnW:%d PC:%04X A:%02X X:%02X Y:%02X SP:%02X P:%02X IR:%02X",
		   cycle,
		   clk,
		   a,
		   d,
		   r_w,
		   readPC(state),
		   readA(state),
		   readX(state),
		   readY(state),
		   readSP(state),
		   readP(state),
		   ir);

	if (clk) {
		if (r_w)
		printf(" R$%04X=$%02X", a, memory[a]);
		else
		printf(" W$%04X=$%02X", a, d);
	}
	printf("\n");
}
