/*
 * util.h
 *
 *  Created on: Apr 30, 2012
 *      Author: alpsayin
 */

#ifndef UTIL_H_
#define UTIL_H_

#define BUILD __DATE__" "__TIME__
#define GMT_OFFSET (+1)

#if 1
#define CHECKPOINT(x) do{ printf("===== CHECKPOINT #%d ===== %s()\n", x, __FUNCTION__); }while(0)
#else
#define CHECKPOINT(x) ;;
#endif

uint8_t text_to_ip(uint8_t* in_and_out, uint8_t in_length);
uint8_t readnline(FILE* fptr, uint8_t* out, uint8_t length);
void print_callsign(uint8_t* callsign);
void print_addr_hex(uint8_t* addr);
void print_addr_dec(uint8_t* addr);

#endif /* UTIL_H_ */
