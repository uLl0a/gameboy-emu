#ifndef GBEMU_CPU_CPU_H_
#define GBEMU_CPU_CPU_H_
#include "useful_utils.h"
#include <stdio.h>
#include <stdbool.h>

typedef unsigned char _u8;
typedef char _s8;
typedef unsigned short _u16;
typedef short _s16;

#define CLOCK_SPEED 4194304 
#define MAX_CICLES (CLOCK_SPEED / FRAME_PER_SECOND)

#define CPU_FLAG_Z 7 /* Zero Flag*/
#define CPU_FLAG_N 6 /* Add/Sub-Flag (BCD) */
#define CPU_FLAG_H 5 /* Half Carry Flag (BCD) */
#define CPU_FLAG_C 4 /* Carry Flag */
#define CPU_FLAG_NZ 14
#define CPU_FLAG_NC 28

typedef struct 
{
    _u16 reg ;
    struct
    {
        _u8 hi ; // A,B,D,H Registers
        _u8 lo ;  // F??,C,E,L Registers
    };
}Register_t;

typedef struct {
     Register_t af;
     Register_t bc;
     Register_t de;
     Register_t hl;

     Register_t sp;
     Register_t pc;
} cpu_registers_t;

struct cpu_context_t{
    cpu_registers_t regs;
    _u8 cur_opcode;
    int cycles;
    bool halted;
    bool stoped;
    bool pending_for_interupt_disable;
    bool pending_for_interupt_enable;
};

extern struct cpu_context_t cpu;

void cpu_reset(void);
void cpu_fetch_opcode(void);
void cpu_decode_and_execute(void);
void cpu_decode_and_execute_extended_opcode(void);
void cpu_step(void);

#endif  // GBEMU_CPU_CPU_H_
