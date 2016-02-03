
#ifdef __cplusplus

struct state_t;

extern "C" {

#else

#ifndef INCLUDED_FROM_NETLIST_SIM_C
#define state_t void
#endif

#endif

extern state_t *initAndResetChip();
extern void delete_chip(state_t*);
extern void step(state_t *state);
extern void chipStatus(state_t *state);
extern unsigned short readPC(state_t *state);
extern unsigned char readA(state_t *state);
extern unsigned char readX(state_t *state);
extern unsigned char readY(state_t *state);
extern unsigned char readSP(state_t *state);
extern unsigned char readP(state_t *state);
extern int readRW(state_t *state);
extern unsigned short readAddressBus(state_t *state);
extern void writeDataBus(state_t *state, unsigned char);
extern unsigned char readDataBus(state_t *state);
extern unsigned char readIR(state_t *state);

extern unsigned char memory[65536];
extern unsigned int cycle;
extern unsigned int transistors;

#ifdef __cplusplus
}
#endif
