#include "LR35902.h"
#include "bus.h"

void _16bit_ld(_u16 *n, _u16 nn,_u16 length, int cycles){
    cpu.cycles += cycles;
    *n = nn;
    cpu.regs.pc.reg += length;
}

void _jr(_s8 nn,_u8 cc,_u16 length , int cycles){
    cpu.cycles += cycles;
    if(cc == 0)
        goto jump;

    switch (cc) {
        case CPU_FLAG_NZ:
            if(!(_CPU_TEST_BIT(CPU_FLAG_Z))){
                goto jump;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_Z:
            if(_CPU_TEST_BIT(CPU_FLAG_Z)){
                goto jump;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_NC:
            if(!(_CPU_TEST_BIT(CPU_FLAG_C))){
                goto jump;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_C:
            if(_CPU_TEST_BIT(CPU_FLAG_C)){
                goto jump;
            }else{
                goto not_taken;
            }
            break;
    }

jump:
    cpu.regs.pc.reg += nn;
    goto end;
not_taken:
    cpu.regs.pc.reg += length;
end:
    return;
}

void _jp(_u16 nn,_s8 cc,_u16 length, int cycles){
    cpu.cycles += cycles;
    if(cc == 0)
        goto jump;

    switch (cc) {
        case CPU_FLAG_NZ:
            if(!(_CPU_TEST_BIT(CPU_FLAG_Z))){
                goto jump;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_Z:
            if(_CPU_TEST_BIT(CPU_FLAG_Z)){
                goto jump;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_NC:
            if(!(_CPU_TEST_BIT(CPU_FLAG_C))){
                goto jump;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_C:
            if(_CPU_TEST_BIT(CPU_FLAG_C)){
                goto jump;
            }else{
                goto not_taken;
            }
            break;
    }

jump:
    cpu.regs.pc.reg = nn;
    goto end;
not_taken:
    cpu.regs.pc.reg += length;
end:
    return;
}

void _call(_u16 nn,_s8 cc,_u16 length, int cycles){
    cpu.cycles += cycles / 2;

    if(cc == 0){
        goto call;
    }

    switch (cc) {
        case CPU_FLAG_NZ:
            if(!(_CPU_TEST_BIT(CPU_FLAG_Z))){
                goto call;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_Z:
            if(_CPU_TEST_BIT(CPU_FLAG_Z)){
                goto call;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_NC:
            if(!(_CPU_TEST_BIT(CPU_FLAG_C))){
                goto call;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_C:
            if(_CPU_TEST_BIT(CPU_FLAG_C)){
                goto call;
            }else{
                goto not_taken;
            }
            break;
    }

call:
    _push(cpu.regs.pc.reg,0, cycles / 2);
    cpu.regs.pc.reg = nn;
    goto end;
not_taken:
    cpu.regs.pc.reg += length;
end:
    return;

}

void _ret(_s8 cc,_u16 length, int cycles){
    cpu.cycles = cycles / 2;
    if(cc == 0)
        goto ret;

    switch (cc) {
        case CPU_FLAG_NZ:
            if(!(_CPU_TEST_BIT(CPU_FLAG_Z))){
                goto ret;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_Z:
            if(_CPU_TEST_BIT(CPU_FLAG_Z)){
                goto ret;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_NC:
            if(!(_CPU_TEST_BIT(CPU_FLAG_C))){
                goto ret;
            }else{
                goto not_taken;
            }
            break;
        case CPU_FLAG_C:
            if(_CPU_TEST_BIT(CPU_FLAG_C)){
                goto ret;
            }else{
                goto not_taken;
            }
            break;
    }

ret:{
        _u16 nn = _pop(0,cycles / 2);
        cpu.regs.pc.reg = nn;
        goto end;
    }
not_taken:
    cpu.regs.pc.reg += length;
end:
    return;
}

void _xor(_u8 *n,_u16 length, int cycles){
    cpu.cycles += cycles;
    cpu.regs.af.hi = (*n ^ cpu.regs.af.hi);
    if(cpu.regs.af.hi == 0){
        _CPU_SET_BIT(CPU_FLAG_Z);
    }
    _CPU_RESET_BIT(CPU_FLAG_N);
    _CPU_RESET_BIT(CPU_FLAG_H);
    _CPU_RESET_BIT(CPU_FLAG_C);
    cpu.regs.pc.reg += length;
}

void _or(_u8 *n,_u16 length, int cycles){
    cpu.cycles += cycles;
    cpu.regs.af.hi = (*n | cpu.regs.af.hi);
    if(cpu.regs.af.hi == 0){
        _CPU_SET_BIT(CPU_FLAG_Z);
    }
    _CPU_RESET_BIT(CPU_FLAG_N);
    _CPU_RESET_BIT(CPU_FLAG_H);
    _CPU_RESET_BIT(CPU_FLAG_C);
    cpu.regs.pc.reg += length;
}

void _and(_u8 *n,_u16 length, int cycles){
    cpu.cycles += cycles;
    cpu.regs.af.hi = (*n & cpu.regs.af.hi);
    if(cpu.regs.af.hi == 0){
        _CPU_SET_BIT(CPU_FLAG_Z);
    }
    _CPU_RESET_BIT(CPU_FLAG_N);
    _CPU_SET_BIT(CPU_FLAG_H);
    _CPU_RESET_BIT(CPU_FLAG_C);
    cpu.regs.pc.reg += length;
}

void _cp(_u8 n,_u16 length, int cycles){
    cpu.cycles += cycles;

    if((cpu.regs.af.hi - n) == 0)
        _CPU_SET_BIT(CPU_FLAG_Z);

    _CPU_SET_BIT(CPU_FLAG_N);

    if(_8BIT_HALF_BORROW(cpu.regs.af.hi, n)){
        _CPU_SET_BIT(CPU_FLAG_H);
    }

    if(cpu.regs.af.hi < n){
        _CPU_RESET_BIT(CPU_FLAG_C);
    }
    cpu.regs.pc.reg += length;
}

void _8bit_ld(_u8 *n, _u8 A,_u16 length, int cycles){
    cpu.cycles += cycles;
    *n = A;
    cpu.regs.pc.reg += length;
}

void _16bit_inc(_u16 *n,_u16 length, int cycles){
    cpu.cycles += cycles;
    *n += 1;
    cpu.regs.pc.reg += length;
}

void _16bit_dec(_u16 *nn,_u16 length, int cycles){
    cpu.cycles += cycles;
    *nn -= 1;
    cpu.regs.pc.reg += length;
}

void _8bit_dec(_u8 *n,_u16 length, int cycles){
    cpu.cycles += cycles;
    _u8 before = *n;
    *n -= 1;
    if(*n == 0)
        _CPU_SET_BIT(CPU_FLAG_Z);

    _CPU_SET_BIT(CPU_FLAG_N);

    if(_8BIT_HALF_BORROW(before, 1))
        _CPU_SET_BIT(CPU_FLAG_H);

    cpu.regs.pc.reg += length;
}

void _8bit_inc(_u8 *n,_u16 length, int cycles){
    cpu.cycles += cycles;
    _u8 before = *n;
    *n += 1;
    if(*n == 0)
        _CPU_SET_BIT(CPU_FLAG_Z);

    _CPU_RESET_BIT(CPU_FLAG_N);

    if(_8BIT_HALF_CARRY(before, 1))
        _CPU_SET_BIT(CPU_FLAG_H);

    cpu.regs.pc.reg += length;
}

void _8bit_memory_inc(_u16 address,_u16 length, int cycles){
    cpu.cycles += cycles;
    _u8 *n = bus_read(address);
    _u8 before = *n;
    *n += 1;
    bus_write(address, *n);
    if(*n == 0)
        _CPU_SET_BIT(CPU_FLAG_Z);

    _CPU_RESET_BIT(CPU_FLAG_N);

    if(_8BIT_HALF_CARRY(before, 1))
        _CPU_SET_BIT(CPU_FLAG_H);

    cpu.regs.pc.reg += length;
}

void _8bit_memory_dec(_u16 address,_u16 length, int cycles){
    cpu.cycles += cycles;
    _u8 *before = bus_read(address);
    _u8 n = *before;
    n -= 1;
    bus_write(address, n);

    if(n == 0)
        _CPU_SET_BIT(CPU_FLAG_Z);

    _CPU_SET_BIT(CPU_FLAG_N);

    if(_8BIT_HALF_BORROW(*before, 1))
        _CPU_SET_BIT(CPU_FLAG_H);

    cpu.regs.pc.reg += length;
}

void _push(_u16 value, _u16 length, int cycles){
    _16bit_bus_write(cpu.regs.sp.reg, value);
    cpu.regs.sp.reg -= 2;
    cpu.regs.pc.reg += length;
    cpu.cycles += cycles;
}

_s16 _pop(_u16 length,int cycles){
    cpu.cycles += cycles;
    _s16 n = (_s16)_16bit_bus_read(cpu.regs.sp.reg);
    cpu.regs.sp.reg += 2;
    cpu.regs.pc.reg += 1;
    return n;
}

void _8bit_add(_u8 *A, _u8 n,_u16 length, int cycles){
    _u8 before = *A;
    *A += n;
    if(*A == 0){
        _CPU_SET_BIT(CPU_FLAG_Z);
    }
    _CPU_RESET_BIT(CPU_FLAG_N);
    if(_8BIT_HALF_CARRY(before, n)){
        _CPU_SET_BIT(CPU_FLAG_H);
    }

    if(_8BIT_CARRY(before,n)){
        _CPU_SET_BIT(CPU_FLAG_C);
    }
    cpu.regs.pc.reg += length;
    cpu.cycles += cycles;
}

void _16bit_add(_u16 *A, _u16 n,_u16 length, int cycles){
    _u16 before = *A;
    *A += n;
    _CPU_RESET_BIT(CPU_FLAG_N);
    if(_16BIT_HALF_CARRY(before, n)){
        _CPU_SET_BIT(CPU_FLAG_H);
    }

    if(_16BIT_CARRY(before,n)){
        _CPU_SET_BIT(CPU_FLAG_C);
    }
    cpu.regs.pc.reg += length;
    cpu.cycles += cycles;
}

void _8bit_sub(_u8 *A, _u8 n,_u16 length, int cycles){
    _u8 before = *A;
    *A -= n;
    if(*A == 0){
        _CPU_SET_BIT(CPU_FLAG_Z);
    }
    _CPU_SET_BIT(CPU_FLAG_N);
    
    if(_8BIT_HALF_BORROW(before, n)){
        _CPU_SET_BIT(CPU_FLAG_H);
    }
    if(_8BIT_BORROW(before, n)){
        _CPU_SET_BIT(CPU_FLAG_C);
    }
    cpu.regs.pc.reg += length;
    cpu.cycles += cycles;
}

void _ccf(){
    cpu.regs.pc.reg += 1;
    cpu.cycles += 14;
    if(_CPU_TEST_BIT(CPU_FLAG_C)){
        _CPU_RESET_BIT(CPU_FLAG_C);
    }else{
        _CPU_SET_BIT(CPU_FLAG_C);
    }
    _CPU_RESET_BIT(CPU_FLAG_N);
    _CPU_RESET_BIT(CPU_FLAG_H);
}

void _scf(){
    cpu.regs.pc.reg += 1;
    cpu.cycles += 14;
    _CPU_RESET_BIT(CPU_FLAG_N);
    _CPU_RESET_BIT(CPU_FLAG_H);
    _CPU_SET_BIT(CPU_FLAG_C);
}

void _stop(){
    cpu.cycles += 4;
    cpu.regs.pc.reg += 2;
    cpu.stoped = true;
}

void _halted(){
    cpu.cycles += 4;
    cpu.regs.pc.reg += 1;
    cpu.halted = true;
}

void _di(){
    cpu.cycles += 4;
    cpu.regs.pc.reg += 1;
    cpu.pending_for_interupt_disable = true;
}

void _ei(){
    cpu.cycles += 4;
    cpu.regs.pc.reg += 1;
    cpu.pending_for_interupt_enable = true;
}

void _rst(_u8 value){
    _push(cpu.regs.pc.reg, 0, 0);
    cpu.cycles += 16;
    cpu.regs.pc.reg = value;
}

