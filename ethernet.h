/* 
 * File:   ethernet.h
 * Author: alpsayin
 *
 * Created on March 21, 2012, 2:03 PM
 */

#ifndef ETHERNET_H
#define	ETHERNET_H

#include <inttypes.h>
#include <stdint.h>
#include "udp_ip.h"

/*! this definition is used to check the lengths of the payloads while creating packets */
#define ETH_MAX_PAYLOAD_LENGTH (UDP_MAX_PAYLOAD_LENGTH+UDP_TOTAL_HEADERS_LENGTH)
#define ETH_TOTAL_HEADERS_LENGTH (18)

/*! used for calculating the offsets in the packet */
#define ETH_DESTINATION_LENGTH 6
/*! used for calculating the offsets in the packet */
#define ETH_SOURCE_LENGTH 6
/*! used for calculating the offsets in the packet */
#define ETH_LENGTH_LENGTH 2
/*! used for calculating the offsets in the packet */
#define ETH_FCS_LENGTH 4

/*! location of destination in an ethernet packet */
#define ETH_DESTINATION_OFFSET 0
/*! location of source in an ethernet packet */
#define ETH_SOURCE_OFFSET (ETH_DESTINATION_OFFSET+ETH_DESTINATION_LENGTH)
/*! location of length field in an ethernet packet */
#define ETH_LENGTH_OFFSET (ETH_SOURCE_OFFSET+ETH_SOURCE_LENGTH)
/*! location of payload in an ethernet packet */
#define ETH_PAYLOAD_OFFSET (ETH_LENGTH_OFFSET+ETH_LENGTH_LENGTH)
/*! location of FCS field in an ethernet packet
 *\param payload_len the length of the payload in the packet
 * */
#define ETH_FCS_OFFSET(payload_len) (ETH_PAYLOAD_OFFSET+payload_len)

#ifdef	__cplusplus
extern "C" {
#endif

	/*!
	 * 	eth_initialize_network()
	 * 	copies the ethernet address to static local eth address
	 */
	void eth_initialize_network(uint8_t* myEth);

	/*!
	 * eth_get_local_address()
	 * return a pointer to the static local address
	 * also if the parameter is not NULL, copies the address to parameter pointer
	 */
    uint8_t* eth_get_local_address(uint8_t* eth_out);

	/*!
	 * eth_get_broadcast_address()
	 * return a pointer to the static broadcast address
	 * also if the parameter is not NULL, copies the address to parameter pointer
	 */
    uint8_t* eth_get_broadcast_address(uint8_t* eth_out);
    /*!
     * eth_create_packet()
     * prepares an ethernet packet with source address, target destination address and payload and puts it in packet_out
     * also computes the checksum and also puts it into the packet
     * on successful encapsulation function returns the length of the packet
     * else returns zero
     */
    uint32_t eth_create_packet(uint8_t* src_in, uint8_t* dst_in, uint8_t* payload_in, uint16_t payload_length, uint8_t* packet_out);

    /*!
     * eth_check_destination()
     * checks the destination of the packet_in with my_dst
     * if packet_dst is not null pointer writes the packet's destination to packet_dst
     * returns zero if the addresses match
     */
    uint8_t eth_check_destination(uint8_t* my_dst, uint8_t* packet_dst, uint8_t* packet_in);

    /*!
     * eth_open_packet()
     * opens the packet_in and writes source address to src_out, destination address to dst_out
     * writes the payload to payload_out
     * before writing anything it first checks the checksum, if the checksum doesn't match,
     * null is written to all pointers and function returns 0
     * on a successful opening function returns the length of the packet
     */
    uint16_t eth_open_packet(uint8_t* src_out, uint8_t* dst_out, uint8_t* payload_out, uint8_t* packet_in, uint16_t packet_length);

#ifdef	__cplusplus
}
#endif

#endif	/* ETHERNET_H */

