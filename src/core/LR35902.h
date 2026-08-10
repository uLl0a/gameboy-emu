#ifndef GBEMU_CPU_LR35902_H_
#define GBEMU_CPU_LR35902_H_
#include "cpu.h"

void _jp(_u16 nn,_s8 cc,_u16 length, int cycles);
void _jr(_s8 nn,_u8 cc,_u16 length , int cycles);
void _call(_u16 nn,_s8 cc,_u16 length, int cycles);
void _ret(_s8 cc,_u16 length, int cycles);
void _cp(_u8 n,_u16 length, int cycles);

void _16bit_ld(_u16 *n, _u16 nn,_u16 length, int cycles);
void _16bit_inc(_u16 *n,_u16 length, int cycles);
void _16bit_dec(_u16 *nn,_u16 length, int cycles);

void _8bit_ld(_u8 *n, _u8 A,_u16 length, int cycles);
void _8bit_dec(_u8 *n,_u16 length, int cycles);
void _8bit_memory_inc(_u16 address,_u16 length, int cycles);
void _8bit_memory_dec(_u16 address,_u16 length, int cycles);
void _8bit_inc(_u8 *n,_u16 length, int cycles);

void _or(_u8 *n,_u16 length, int cycles);
void _and(_u8 *n,_u16 length, int cycles);
void _xor(_u8 *n,_u16 length, int cycles);

void _push(_u16 value, _u16 length, int cycles);
_s16 _pop(_u16 length,int cycles);

void _8bit_add(_u8 *A, _u8 n,_u16 length, int cycles);
void _16bit_add(_u16 *A, _u16 n,_u16 length, int cycles);
void _8bit_sub(_u8 *A, _u8 n,_u16 length, int cycles);
void _rst(_u8 value);

void _ccf();
void _scf();
void _halted();
void _stop();
void _di();
void _ei();
#endif //GBEMU_CPU_LR35902_H_
