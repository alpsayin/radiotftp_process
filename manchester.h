/* 
 * File:   manchester.h
 * Author: alpsayin
 *
 * Created on March 15, 2012, 7:46 PM
 */

#ifndef MANCHESTER_H
#define	MANCHESTER_H

#include <inttypes.h>
#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

uint16_t manchester_encode(uint8_t* input, uint8_t* output, uint16_t size);
uint16_t manchester_decode(uint8_t* input, uint8_t* output, uint16_t size);
uint8_t isManchester_encoded(uint8_t);

#ifdef	__cplusplus
}
#endif

#endif	/* MANCHESTER_H */

