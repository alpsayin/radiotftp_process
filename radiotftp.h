/* 
 * File:   radiotftp.h
 * Author: alpsayin
 *
 * Created on April 9, 2012, 8:36 PM
 */

#ifndef RADIOTFTP_H
#define	RADIOTFTP_H

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#include "manchester.h"
#include "ethernet.h"
#include "udp_ip.h"
#include "tftp.h"
#include "radiotftp.h"
#include "ax25.h"
#include "contiki.h"
#include "contiki-conf.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#define RADIOTFTP_ENABLE_ACKNOWLEDGMENTS 1
#define REMOTE_FILENAME "sensors.dat"
#define APPEND 1
#define HELLO_WORLD_PORT 12345
#define END_OF_FILE 28 //do not change
#define AX25_ENABLED 1
#define ETHERNET_ENABLED 0
#define MY_AX25_CALLSIGN "SA0BXI\x0f"
#define MY_ETHERNET_ADDRESS	{0xf0, 0x0, 0x0, 0x0, 0x0, 0x1}
#define MY_IP_ADDRESS { 0xa1, 0xa2, 0xa3, 0xa4 }

void radiotftpAlarm_callback(void* data);
int uart0_rx(unsigned char receivedByte);
int uart1_rx(unsigned char receivedByte);
uint8_t setRTS(uint8_t level);
void radiotftp_setNumBytesToSend(uint16_t numBytes);
uint16_t radiotftp_getNumBytesToSend();
uint8_t queueSerialData(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* dataptr, uint16_t datalen);
uint16_t transmitSerialData(void);
uint8_t udp_packet_demultiplexer(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* payload, uint16_t len);
PROCESS_NAME(radiotftp_process);

#endif
