/* 
 * File:   timers.h
 * Author: alpsayin
 *
 * Created on April 6, 2012, 3:48 PM
 */

#ifndef TIMERS_H
#define	TIMERS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>

#define TIMER_HANDLER_FUNCTION_PROTO( timerHandler) uint8_t timerHandler()
#define TIMER_HANDLER_FUNCTION( timerHandler) uint8_t timerHandler()

    typedef uint8_t (*timerHandlerfptr_t)(void*);

    uint8_t timers_initialize(void(*handlerfptr)(void*));
    uint8_t timers_create_timer( int expireS, int expireMS);
    uint8_t timers_cancel_timer(void);

#ifdef	__cplusplus
}
#endif

#endif	/* TIMERS_H */

