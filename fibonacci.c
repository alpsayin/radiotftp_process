/*
 * fibonacci.c
 *
 *  Created on: Jun 6, 2012
 *      Author: alpsayin
 */

#include <stdio.h>
#include <avr/io.h>
#include <avr/iom128rfa1.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "contiki.h"
#include "contiki-conf.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "dev/rs232.h"
#include "radiotftp.h"

PROCESS(measurement_process, "Measurement Process");
AUTOSTART_PROCESSES(&measurement_process, &radiotftp_process);

PROCESS_THREAD(measurement_process, ev, data)
{
	static uint32_t counter=0;
	static uint16_t numBytes=0;
	static uint64_t fibo[3] = {0, 1, 1};
	static struct etimer measurement_timer;
	static uint8_t fake_measurement_string[450];
	PROCESS_BEGIN();

	etimer_set(&measurement_timer, CLOCK_SECOND*2);
	while(1)
	{
		PROCESS_WAIT_EVENT();
		counter++;
		fibo[2]=fibo[1]+fibo[0];
		numBytes=sprintf(fake_measurement_string, "Some Data: fibonacci(%d)=", counter);
		numBytes+=sprintf(fake_measurement_string+numBytes, "%u [Alp Sayin, KTH Royal Institute of Technology]\n", fibo[0]);
		printf("%s", fake_measurement_string);
		fibo[0]=fibo[1];
		fibo[1]=fibo[2];
		radiotftp_setNumBytesToSend(numBytes);
		process_post_synch(&radiotftp_process, PROCESS_EVENT_COM, (void*)fake_measurement_string);
		etimer_set(&measurement_timer, CLOCK_SECOND*10);
	}
	PROCESS_END();
}

