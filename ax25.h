/*
 * ax25.h
 *
 *  Created on: Apr 19, 2012
 *      Author: alpsayin
 */

#ifndef AX25_H_
#define AX25_H_

#include <inttypes.h>
#include <stdint.h>
#include "udp_ip.h"

/*
 * according to the standards below definition should be 256, but we ignored a lot of rules
 * including bit stuffing and uart framing, so this is just another one of them
 */
#define AX25_MAX_PAYLOAD_LENGTH (UDP_MAX_PAYLOAD_LENGTH+UDP_TOTAL_HEADERS_LENGTH)
#define AX25_TOTAL_HEADERS_LENGTH (18)

#define AX25_CONTROL_UI_FINAL	0x03
#define AX25_CONTROL_UI_POLL	0x13
#define AX25_PID_NO_PROTOCOL	0xF0
#define AX25_PID_COMPRESSED_TCP	0x06
#define AX25_PID_UNCOMPRESSED_TCP	0x07

#define AX25_DESTINATION_LENGTH 7
#define AX25_SOURCE_LENGTH 7
#define AX25_CONTROL_LENGTH 1
#define AX25_PID_LENGTH 1
#define AX25_FCS_LENGTH 2

#define AX25_DESTINATION_OFFSET 0
#define AX25_SOURCE_OFFSET (AX25_DESTINATION_OFFSET+AX25_DESTINATION_LENGTH)
#define AX25_CONTROL_OFFSET (AX25_SOURCE_OFFSET+AX25_SOURCE_LENGTH)
#define AX25_PID_OFFSET (AX25_CONTROL_OFFSET+AX25_CONTROL_LENGTH)
#define AX25_PAYLOAD_OFFSET (AX25_PID_OFFSET+AX25_PID_LENGTH)
#define AX25_FCS_OFFSET(payload_len) (AX25_PAYLOAD_OFFSET+payload_len)

	/*!
	 * 	ax25_initialize_network()
	 * 	copies the ax25 callsign to static local eth address
	 */
	void ax25_initialize_network(uint8_t* myCallsign);

	/*!
	 * ax25_get_local_address()
	 * return a pointer to the static local address
	 * also if the parameter is not NULL, copies the callsign to parameter pointer
	 */
    uint8_t* ax25_get_local_callsign(uint8_t* callsign_out);

	/*!
	 * ax25_get_broadcast_callsign()
	 * return a pointer to the static broadcast address
	 * also if the parameter is not NULL, copies the callsign to parameter pointer
	 */
    uint8_t* ax25_get_broadcast_callsign(uint8_t* callsign_out);

    /*!
     * ax25_create_packet()
     * prepares an ax25 packet with source address, target destination address and payload and puts it in packet_out
     * also computes the checksum and also puts it into the packet
     * on successful encapsulation function returns the length of the packet
     * else returns zero
     */
    uint32_t ax25_create_ui_packet(uint8_t* src_in, uint8_t* dst_in, uint8_t* payload_in, uint16_t payload_length, uint8_t* packet_out);
    /*!
     * ax25_check_destination()
     * checks the destination of the packet_in with my_dst
     * if packet_dst is not null pointer writes the packet's destination to packet_dst
     * returns zero if the addresses match
     */
    uint8_t ax25_check_destination(uint8_t* my_dst, uint8_t* packet_dst_out, uint8_t* packet_in);
    /*!
     * ax25_open_packet()
     * opens the packet_in and writes source address to src_out, destination address to dst_out
     * writes the payload to payload_out
     * before writing anything it first checks the checksum, if the checksum doesn't match,
     * null is written to all pointers and function returns 0
     * on a successful opening function returns the length of the packet
     */
    uint16_t ax25_open_ui_packet(uint8_t* src_out, uint8_t* dst_out, uint8_t* payload_out, uint8_t* packet_in, uint16_t packet_length);




#endif /* AX25_H_ */
