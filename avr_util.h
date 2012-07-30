
#ifndef AVR_UTIL_H
#define AVR_UTIL_H

#include <stdio.h>

#define SET_BIT(port, bit)    ((port) |= _BV(bit))
#define CLR_BIT(port, bit)    ((port) &= ~_BV(bit))
#define READ_BIT(port, bit)   (((port) & _BV(bit)) != 0)
#define FLIP_BIT(port, bit)   ((port) ^= _BV(bit))
#define WRITE_BIT(port, bit, value) \
  if (value) SET_BIT((port), (bit)); \
  else CLR_BIT((port), (bit))

// Bit operators using bit flag mask
#define SET_FLAG(port, flag)  ((port) |= (flag))
#define CLR_FLAG(port, flag)  ((port) &= ~(flag))
#define READ_FLAG(port, flag) ((port) & (flag))

#define PRINT_REG(x) msgLen = sprintf(msgBuffer, "[%p] = 0x%X\n\0", &x ,x); puts(msgBuffer);

#define STRLEN(s) ((sizeof(s)/sizeof(s[0]))-1)
#define HIGH(x) ((x&0xF0)>>4)
#define LOW(x) (x&0x0F)

#define ATOMIC_BEGIN() cli()
#define ATOMIC_END() sei()
#define ATOMIC_SET(dst, src) {ATOMIC_BEGIN(); dst=src; ATOMIC_END();}

#if 1
#define PRINTF_D(...) printf(__VA_ARGS__)
#else
#define PRINTF_D(...)
#endif


#endif
