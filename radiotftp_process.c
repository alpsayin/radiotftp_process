#include <stdio.h>
#include <avr/io.h>
#include <avr/iom128rfa1.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/wdt.h>

#include "contiki.h"
#include "contiki-conf.h"
#include "contiki-net.h"
#include "contiki-lib.h"
#include "dev/rs232.h"

#include "manchester.h"
#include "ethernet.h"
#include "udp_ip.h"
#include "tftp.h"
#include "radiotftp.h"
#include "ax25.h"
#include "util.h"
#include "avr_util.h"
#include "printAsciiHex.h"

const uint8_t my_ip_address[6] = MY_IPv6_ADDRESS;

#define PREAMBLE_LENGTH 10
unsigned char preamble[PREAMBLE_LENGTH] =
{ 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };

#define SYNC_LENGTH 4
unsigned char syncword[SYNC_LENGTH] =
{ 0xAA, 0x55, 0xAA, 0x55 };

#if ETHERNET_ENABLED==1
const uint8_t my_eth_address[6] = MY_ETHERNET_ADDRESS;
static uint8_t ethernet_buffer[ETH_MAX_PAYLOAD_LENGTH + ETH_TOTAL_HEADERS_LENGTH];
static uint8_t manchester_buffer[sizeof(ethernet_buffer) * 2];
#elif AX25_ENABLED==1
const uint8_t my_ax25_callsign[7] = MY_AX25_CALLSIGN;
static uint8_t ax25_buffer[AX25_MAX_PAYLOAD_LENGTH+AX25_TOTAL_HEADERS_LENGTH];
static uint8_t manchester_buffer[sizeof(ax25_buffer)*2];
#else
uint8_t manchester_buffer[(UDP_MAX_PAYLOAD_LENGTH + UDP_TOTAL_HEADERS_LENGTH) * 2];
#endif
static uint8_t udp_buffer[UDP_MAX_PAYLOAD_LENGTH+20];

static uint8_t transmit_buffer[sizeof(manchester_buffer)+PREAMBLE_LENGTH+SYNC_LENGTH+2];
static uint16_t transmit_length = 0;

static uint8_t io[sizeof(manchester_buffer)+PREAMBLE_LENGTH+SYNC_LENGTH+2];
static uint16_t io_index = 0;
static uint16_t saved_io_index = 0;
static uint8_t sync_counter = 0;
static uint8_t sync_passed = 0;

volatile uint8_t io_flag = 0;
volatile uint8_t alarm_flag = 0;
volatile uint8_t timer_flag = 0;
volatile uint8_t queue_flag = 0;
volatile uint16_t numBytesToSend = 0;

static uint8_t udp_src[6], udp_dst[6];
static uint16_t udp_src_prt, udp_dst_prt;

#if PREAMBLE_LENGTH > 15
#error preamble length cant be longer than 15
#endif

#if AX25_ENABLED==1 && ETHERNET_ENABLED==1
#error Both AX25 and Ethernet cannot be enabled
#endif

PROCESS(radiotftp_process, "Radiotftp Process");

void radiotftpAlarm_callback(void* data)
{
	//PRINTF_D("main timer handler\n");
	timer_flag = 1;
	process_post(&radiotftp_process, PROCESS_EVENT_TIMER, NULL);
}

int uart0_rx(unsigned char receivedByte)
{
	//stdin
	rs232_send(RS232_PORT_0, receivedByte);
	return 0;
}
int uart1_rx(unsigned char receivedByte)
{
	//radiometrix
//	putchar(receivedByte);
	if(sync_passed)
	{
		if(receivedByte==END_OF_FILE || !isManchester_encoded(receivedByte) )
		{
			putchar('\n');
			sync_passed = 0;
			saved_io_index = io_index;
			io_flag=1;
			if(process_post(&radiotftp_process, PROCESS_EVENT_COM, (void*) io)==PROCESS_ERR_FULL)
			{
				printf("incoming transmission discarded\n");
			}
		}
		else
		{
			io[io_index] = receivedByte;
			io_index = ((io_index+1)%sizeof(io));
		}
	}
	else
	{
		if(receivedByte==syncword[sync_counter])
		{
			//one more step closer to
			sync_counter++;
			//PRINTF_D("sync counting=%d\n", sync_counter);
			if(sync_counter==SYNC_LENGTH)
			{
				io_index=0;
				sync_passed = 1;
				sync_counter = 0;
				//PRINTF_D("sync passed\n");
			}
		}
		else
		{
			//reset the sync counter
			sync_counter = 0;
		}
	}
	return 0;
}

uint8_t setRTS(uint8_t level)
{
	if(level)
		SET_BIT(PORTF, 0);
	else
		CLR_BIT(PORTF, 0);
	return READ_BIT(PORTF, 0);
}

void radiotftp_setNumBytesToSend(uint16_t numBytes)
{
//	PRINTF_D("setting num of bytes to send %d\n", numBytes);
	ATOMIC_SET(numBytesToSend, numBytes);
}

uint16_t radiotftp_getNumBytesToSend()
{
	return numBytesToSend;
}

uint8_t queueSerialData(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* dataptr, uint16_t datalen)
{
	uint16_t idx = 0, len = 0;

	wdt_reset();
	if(queue_flag)
	{
		return -1;
	}

	memcpy(transmit_buffer, preamble, PREAMBLE_LENGTH);
	idx += PREAMBLE_LENGTH;

	memcpy(transmit_buffer+idx, syncword, SYNC_LENGTH);
	idx += SYNC_LENGTH;

	//PRINTF_D("udp payload: %s\n", dataptr);
	len = udp_create_packet(src, src_port, dst, dst_port, dataptr, datalen, udp_buffer);
	if(len==0)
	{
		PRINTF_D("couldn't prepare udp packet\n");
		return -2;
	}

#if ETHERNET_ENABLED==1
	//PRINTF_D("eth payload: %s\n", udp_buffer);
	len = eth_create_packet(eth_get_local_address(NULL), eth_get_broadcast_address(NULL), udp_buffer, len, ethernet_buffer);
	if(len==0)
	{
		PRINTF_D("couldn't prepare eth packet\n");
		return -3;
	}
	len = manchester_encode(ethernet_buffer, manchester_buffer, len);
#elif AX25_ENABLED==1
	//PRINTF_D("ax25 payload: %s\n", udp_buffer);
	len = ax25_create_ui_packet(ax25_get_local_callsign(NULL), ax25_get_broadcast_callsign(NULL), udp_buffer, len, ax25_buffer);
	if(len==0)
	{
		PRINTF_D("couldn't prepare ax25 packet\n");
		return -3;
	}
	len = manchester_encode(ax25_buffer, manchester_buffer, len);
#else
	len = manchester_encode(udp_buffer, manchester_buffer, len);
#endif

	memcpy(&transmit_buffer[idx], manchester_buffer, len);
	idx += len;

	transmit_buffer[idx++] = END_OF_FILE;
	transmit_buffer[idx++] = 0;

	queue_flag = 1;
	transmit_length = idx;

	//print_time("data queued");
	wdt_reset();

	if(process_post(&radiotftp_process, PROCESS_EVENT_COM, (void*) io)==PROCESS_ERR_FULL)
	{
		printf("incoming transmission discarded\n");
	}
	
	return 0;
}

uint16_t transmitSerialData(void)
{
	uint16_t i = 0;

	wdt_reset();
	setRTS(0);
	_delay_ms(2e1);

	ATOMIC_BEGIN();
	for(i = 0; i<transmit_length; i++)
	{
		//NOTE: rs232_send has caused problems before
		//while (!READ_BIT(UCSR1A, UDRE1));
		//UDR1 = transmit_buffer[i];
		rs232_send(RS232_PORT_1, transmit_buffer[i]);
		//rs232_send(RS232_PORT_0, transmit_buffer[i]);
	}
	while (!READ_BIT(UCSR1A, UDRE1));
	ATOMIC_END();

	_delay_ms(2e1);
	setRTS(1);
	wdt_reset();


	//print_time("data sent");

	return 0;
}

uint8_t udp_packet_demultiplexer(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* payload, uint16_t len)
{
	//TODO put back the server functions to handle single block messages
//    PRINTF_D("== udp packet handler ==\n");
//    PRINTF_D("== SRC: %02x.%02x.%02x.%02x.%02x.%02x:%d ==\n", src[0], src[1], src[2], src[3], src[4], src[5], src_port);
//    PRINTF_D("== DST: %02x.%02x.%02x.%02x.%02x.%02x:%d ==\n", dst[0], dst[1], dst[2], dst[3], dst[4], dst[5], dst_port);

	uint8_t different = 0;
	wdt_reset();

	//check for address match
	different = memcmp(udp_get_localhost_ip(NULL), dst, IPV6_DESTINATION_LENGTH);
	if(different)
	{
		different = memcmp(udp_get_broadcast_ip(NULL), dst, IPV6_DESTINATION_LENGTH);
	}

	if(!different)
	{
		/* Contiki-Os port does not support tftp-server
		 if(dst_port == 69)
		 {
		 //PRINTF_D("tftp negotiate port\n");
		 tftp_negotiate(src, src_port, dst, dst_port, payload, len - 8);
		 }
		 else */
		if(dst_port==tftp_transfer_src_port())
		{
			//PRINTF_D("tftp negotiate port\n");
			tftp_transfer(src, src_port, dst, dst_port, payload, len-8);
		}
		else if(dst_port==HELLO_WORLD_PORT)
		{
			PRINTF_D("New neighbour:\nIP = %d.%d.%d.%d.%d.%d\n", src[0], src[1], src[2], src[3], src[4], src[5]);
		}
		else
		{
			PRINTF_D("packet received for unassigned port: %d\n", dst_port);
		}
	}
	else
	{
		//nothing to do here yet
		//may be for routing purposes...
		PRINTF_D("packet for someone else: ");
		print_addr_dec(dst);

	}
	return 0;
}
PROCESS_THREAD(radiotftp_process, ev, data)
{
	uint16_t i, temp_io_index;
	int16_t result = 0;
	static struct etimer wait_timer;
	PROCESS_BEGIN()
		;
		PRINTF_D("%s begin\n", PROCESS_CURRENT()->name);

		SET_BIT(DDRF, 0);
		SET_BIT(MCUCR, PUD);
		SET_BIT(DPDS0, PFDRV1);
		SET_BIT(DPDS0, PFDRV0);

		CLR_BIT(DDRE, 0);
		SET_BIT(DDRE, 1);
		SET_BIT(DDRE, 2);
		SET_BIT(DDRE, 3);
		SET_BIT(DDRE, 4);
		CLR_BIT(DDRE, 5);

		SET_BIT(PORTE, 2);
		SET_BIT(PORTE, 3);
		SET_BIT(PORTE, 4);

		rs232_init(RS232_PORT_0, USART_BAUD_38400, USART_PARITY_NONE|USART_STOP_BITS_1|USART_DATA_BITS_8|USART_RECEIVER_ENABLE|USART_INTERRUPT_RX_COMPLETE);
		rs232_set_input(RS232_PORT_0, uart0_rx);

		rs232_init(RS232_PORT_1, USART_BAUD_4800, USART_PARITY_NONE|USART_STOP_BITS_1|USART_DATA_BITS_8|USART_RECEIVER_ENABLE|USART_INTERRUPT_RX_COMPLETE);
		rs232_set_input(RS232_PORT_1, uart1_rx);

		/*while(1)
		 {
		 PROCESS_WAIT_EVENT();
		 if(io_flag)
		 {
		 //PRINTF_D("# of bytes read = %d\n", res);
		 ATOMIC_SET(temp_io_index, io_index);
		 for(i = 0; i < temp_io_index; i++)
		 {
		 putchar(io[i]);
		 }
		 ATOMIC_SET(io_index, 0);
		 ATOMIC_SET(io_flag, 0);
		 }
		 }*/

		timers_initialize(radiotftpAlarm_callback);
#if AX25_ENABLED==1
		ax25_initialize_network(my_ax25_callsign);
		PRINTF_D("AX25 CALLSIGN = ");
		print_callsign(ax25_get_local_callsign(NULL));
#elif ETHERNET_ENABLED==1
		eth_initialize_network(my_eth_address);
		PRINTF_D("Ethernet Address = ");
		print_addr_hex(eth_get_local_address(NULL));
#endif
		udp_initialize_ip_network(my_ip_address, &queueSerialData);
		PRINTF_D("IPv6 Address = ");
		print_addr_dec(udp_get_localhost_ip(NULL));
		tftp_initialize(udp_get_data_queuer_fptr());

		//entering the main while loop
		PRINTF_D("started listening...\n");
		while(1)
		{
			/* usage
			 * radiotftp_setNumBytesToSend(500);
			 * process_post_synch(&radiotftp_process, PROCESS_EVENT_CONTINUE, io);
			 */
			PRINTF_D("waiting for an event/flag\n");
			PROCESS_WAIT_EVENT();
			//PROCESS_WAIT_EVENT_UNTIL(timer_flag || queue_flag || io_flag || numBytesToSend);
			if(numBytesToSend)
			{
				PRINTF_D("starting to send request, numBytes=%d\n", numBytesToSend);
#if RADIOTFTP_ENABLE_ACKNOWLEDGMENTS
				tftp_sendRequest(TFTP_OPCODE_WRQ, udp_get_broadcast_ip(NULL), (uint8_t*) data, numBytesToSend, REMOTE_FILENAME, strlen(REMOTE_FILENAME), APPEND);
#else
				tftp_sendSingleBlockData(udp_get_broadcast_ip(NULL), (uint8_t*)data, numBytesToSend, REMOTE_FILENAME);
#endif
				numBytesToSend = 0;
			}
			if(timer_flag)
			{
				PRINTF_D("timer_flag=1\n");
				tftp_timer_handler();
				timer_flag = 0;
			}
			if(queue_flag)
			{
				if(!sync_passed&&sync_counter<1)
				{
					//usleep(600000ul);
					/*
					 etimer_set(&wait_timer, CLOCK_SECOND * 3 / 5);
					 do
					 {
					 PROCESS_YIELD();
					 } while(ev != PROCESS_EVENT_TIMER);
					 */
					transmitSerialData();
					queue_flag = 0;
				}
			}
			if(io_flag)
			{
				//PRINTF_D("# of bytes read = %d\n", saved_io_index);
				ATOMIC_SET(temp_io_index, saved_io_index);
				result = manchester_decode(io, manchester_buffer, temp_io_index);
#if ETHERNET_ENABLED==1
				result = eth_open_packet(NULL, NULL, ethernet_buffer, manchester_buffer, result);
#elif AX25_ENABLED==1
				result = ax25_open_ui_packet(NULL, NULL, ax25_buffer, manchester_buffer, result);
#else
				result = 1;
#endif
				if(result)
				{
					//PRINTF_D("%s\n",buf);
#if ETHERNET_ENABLED==1
					result = udp_open_packet(udp_src, &udp_src_prt, udp_dst, &udp_dst_prt, udp_buffer, ethernet_buffer);
#elif AX25_ENABLED==1
					result = udp_open_packet(udp_src, &udp_src_prt, udp_dst, &udp_dst_prt, udp_buffer, ax25_buffer);
#else
					result = udp_open_packet(udp_src, &udp_src_prt, udp_dst, &udp_dst_prt, udp_buffer, manchester_buffer);
#endif
					if(result)
					{
						//PRINTF_D("%s\n",buf);
						udp_packet_demultiplexer(udp_src, udp_src_prt, udp_dst, udp_dst_prt, udp_buffer, result);
					}
					else
					{
						PRINTF_D("!udp discarded!\n");
					}
				}
				else
				{
#if ETHERNET_ENABLED==1
					PRINTF_D("!eth discarded!\n");
#elif AX25_ENABLED==1
					PRINTF_D("!ax25_discarded!\n");
#endif
				}
			}
			ATOMIC_SET(io_flag, 0);
		}

	PROCESS_END();
}
