

#include <stdio.h>
#include <string.h>
#include "timers.h"
#include "contiki.h"
#include "contiki-conf.h"
#include "contiki-net.h"
#include "contiki-lib.h"

static struct ctimer alarm_timer;
void (*mainTimerHandler)(void*);

uint8_t timers_initialize( void(*handlerfptr)(void* ))
{
    mainTimerHandler=handlerfptr;
    return 0;
}

uint8_t timers_create_timer( int expireS, int expireMS)
{
	ctimer_set(&alarm_timer, (expireS*CLOCK_SECOND)+(expireMS*CLOCK_SECOND/1000), mainTimerHandler, 0);
	return 0;
}
uint8_t timers_cancel_timer(void)
{
	ctimer_stop(&alarm_timer);
    return 0;
}
