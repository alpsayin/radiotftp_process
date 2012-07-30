/* 
 * File:   udp.h
 * Author: alpsayin
 *
 * Created on March 21, 2012, 2:54 PM
 */

#ifndef UDP_H
#define	UDP_H

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>

#include "ethernet.h"

#define UDP_MAX_PAYLOAD_LENGTH (516) //derived from the tftp requirement
#define UDP_TOTAL_HEADERS_LENGTH (28)

#define IPV6_VERSION_PRIORITY_LENGTH 1
#define IPV6_FLOW_LABEL_LENGTH 3
#define IPV6_PAYLOAD_LENGTH_LENGTH 2
#define IPV6_NEXT_HEADER_LENGTH 1
#define IPV6_HOP_LIMIT_LENGTH 1
#define IPV6_SOURCE_LENGTH 6
#define IPV6_DESTINATION_LENGTH 6

#define IPV6_VERSION_PRIORITY_OFFSET 0
#define IPV6_FLOW_LABEL_OFFSET (IPV6_VERSION_PRIORITY_OFFSET+IPV6_VERSION_PRIORITY_LENGTH)
#define IPV6_PAYLOAD_LENGTH_OFFSET (IPV6_FLOW_LABEL_OFFSET+IPV6_FLOW_LABEL_LENGTH)
#define IPV6_NEXT_HEADER_OFFSET (IPV6_PAYLOAD_LENGTH_OFFSET+IPV6_PAYLOAD_LENGTH_LENGTH)
#define IPV6_HOP_LIMIT_OFFSET (IPV6_NEXT_HEADER_OFFSET+IPV6_NEXT_HEADER_LENGTH)
#define IPV6_SOURCE_OFFSET (IPV6_HOP_LIMIT_OFFSET+IPV6_HOP_LIMIT_LENGTH)
#define IPV6_DESTINATION_OFFSET (IPV6_SOURCE_OFFSET+IPV6_SOURCE_LENGTH)
#define IPV6_PAYLOAD_OFFSET (IPV6_DESTINATION_OFFSET+IPV6_DESTINATION_LENGTH)

#define UDP_SOURCE_PORT_LENGTH 2
#define UDP_DESTINATION_PORT_LENGTH 2
#define UDP_LENGTH_LENGTH 2
#define UDP_CHECKSUM_LENGTH 2

#define UDP_SOURCE_PORT_OFFSET (IPV6_PAYLOAD_OFFSET)
#define UDP_DESTINATION_PORT_OFFSET (UDP_SOURCE_PORT_OFFSET+UDP_SOURCE_PORT_LENGTH)
#define UDP_LENGTH_OFFSET (UDP_DESTINATION_PORT_OFFSET+UDP_DESTINATION_PORT_LENGTH)
#define UDP_CHECKSUM_OFFSET (UDP_LENGTH_OFFSET+UDP_LENGTH_LENGTH)
#define UDP_PAYLOAD_OFFSET (UDP_CHECKSUM_OFFSET+UDP_CHECKSUM_LENGTH)

#define IPV6_HOP_LIMIT 2

#define PACKET_HANDLER_FUNCTION_PROTO( appName) uint8_t appName(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* payload, uint16_t len)
#define PACKET_HANDLER_FUNCTION( appName) uint8_t appName(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* payload, uint16_t len)

#ifdef	__cplusplus
extern "C" {
#endif

    typedef uint8_t (*dataQueuerfptr_t)(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* dataptr, uint16_t datalen);
    typedef uint8_t (*packetHandlerfptr_t)(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* payload, uint16_t len);

    void udp_initialize_ip_network(uint8_t* myIpAddress, dataQueuerfptr_t dataQueuer);

    dataQueuerfptr_t udp_get_data_queuer_fptr(void);

    uint8_t* udp_get_localhost_ip(uint8_t* ip_out);
    uint8_t* udp_get_broadcast_ip(uint8_t* ip_out);

    uint16_t udp_create_packet(uint8_t* src_in, uint16_t src_port, uint8_t* dst_in, uint16_t dst_port, uint8_t* payload_in, uint16_t payload_length, uint8_t* packet_out);

    uint8_t udp_check_destination(uint8_t* my_dst, uint8_t* packet_dst, uint8_t* packet_in);

    uint16_t udp_open_packet(uint8_t* src_out, uint16_t* src_port_out,
                                        uint8_t* dst_out, uint16_t* dst_port_out,
                                        uint8_t* payload_out,
                                        uint8_t* packet_in);

    uint16_t udp_open_packet_extended(uint8_t* src_out, uint16_t* src_port_out,
                                        uint8_t* dst_out, uint16_t* dst_port_out,
                                        uint8_t* payload_out,
                                        uint8_t* packet_in,
                                        uint8_t* flow_label_out,
                                        uint8_t* hop_limit_out,
                                        uint8_t* next_header_out,
                                        uint8_t* version_out,
                                        uint8_t* priority_out);
    

#ifdef	__cplusplus
}
#endif

#endif	/* UDP_H */

