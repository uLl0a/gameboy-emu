#ifndef GBEMU_USEFUL_UTILS_H_
#define GBEMU_USEFUL_UTILS_H_

#define debug_print printf

#define SCREEN_RESOLUTION_X 160
#define SCREEN_RESOLUTION_Y 160
#define FRAME_PER_SECOND 60

#define _MAX(a,b) ((a) > (b) ? (a) : (b))
#define _MIN(a,b) ((a) < (b) ? (a) : (b))

#define _CPU_SET_BIT(b) cpu.regs.af.lo |= ((_u8) 0x0001 << (b))
#define _CPU_TEST_BIT(b) cpu.regs.af.lo &((_u8) 0x0001 << (b))
#define _CPU_RESET_BIT(b) cpu.regs.af.lo &= ~((_u8) 0x0001 << (b))

#define _8BIT_CARRY(a,b) ((a & 0xff) + (b & 0xff) > 0xff)
#define _8BIT_HALF_CARRY(a,b) ((a & 0xf) + (b & 0xf) > 0xf ) 

#define _8BIT_BORROW(a,b) ((a & 0xff) - (b & 0xff) < 0x00)
#define _8BIT_HALF_BORROW(a,b) ((a & 0xf) - (b & 0xf) < 0x00 )

#define _16BIT_CARRY(a,b) ((a & 0xffff) + (b & 0xffff) > 0xffff) 
#define _16BIT_HALF_CARRY(a,b) ((a & 0xff) + (b & 0xff) > 0xff)

#define _16BIT_BORROW(a,b) ((a & 0xffff) - (b & 0xffff) < 0x0000)
#define _16BIT_HALF_BORROW(a,b) ((a & 0xff) - (b & 0xff) < 0x00)

#define _IS_BETWEEN(address, start, end) ((address >= start) && (address <= end))

#endif  // GBEMU_USEFUL_UTILS_H_

