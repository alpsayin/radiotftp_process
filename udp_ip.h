/* 
 * File:   udp.h
 * Author: alpsayin
 *
 * Created on March 21, 2012, 2:54 PM
 */

#ifndef UDP_H
#define UDP_H

#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>

#include "ax25.h"

#define UDP_MAX_PAYLOAD_LENGTH (256)
#define UDP_TOTAL_HEADERS_LENGTH (18+8)

#define IPV4_VERSIONnIHL_LENGTH 1
#define IPV4_DSCPnECN_LENGTH 1
#define IPV4_TOTAL_LENGTH_LENGTH 2
#define IPV4_IDENTIFICATION_LENGTH 2
#define IPV4_FLAGSnFRAGMENT_OFFSET_LENGTH 2
#define IPV4_TIME_TO_LIVE_LENGTH 1
#define IPV4_PROTOCOL_LENGTH 1
#define IPV4_HEADER_CHECKSUM_LENGTH 2
#define IPV4_SOURCE_LENGTH 4
#define IPV4_DESTINATION_LENGTH 4

#define IPV4_VERSIONnIHL_OFFSET 0
#define IPV4_DSCPnECN_OFFSET (IPV4_VERSIONnIHL_OFFSET+IPV4_VERSIONnIHL_LENGTH)
#define IPV4_TOTAL_LENGTH_OFFSET (IPV4_DSCPnECN_OFFSET+IPV4_DSCPnECN_LENGTH)
#define IPV4_IDENTIFICATION_OFFSET (IPV4_TOTAL_LENGTH_OFFSET+IPV4_TOTAL_LENGTH_LENGTH)
#define IPV4_FLAGSnFRAGMENT_OFFSET_OFFSET (IPV4_IDENTIFICATION_OFFSET+IPV4_IDENTIFICATION_LENGTH)
#define IPV4_TIME_TO_LIVE_OFFSET (IPV4_FLAGSnFRAGMENT_OFFSET_OFFSET+IPV4_FLAGSnFRAGMENT_OFFSET_LENGTH)
#define IPV4_PROTOCOL_OFFSET (IPV4_TIME_TO_LIVE_OFFSET+IPV4_TIME_TO_LIVE_LENGTH)
#define IPV4_HEADER_CHECKSUM_OFFSET (IPV4_PROTOCOL_OFFSET+IPV4_PROTOCOL_LENGTH)
#define IPV4_SOURCE_OFFSET (IPV4_HEADER_CHECKSUM_OFFSET+IPV4_HEADER_CHECKSUM_LENGTH)
#define IPV4_DESTINATION_OFFSET (IPV4_SOURCE_OFFSET+IPV4_SOURCE_LENGTH)
#define IPV4_PAYLOAD_OFFSET (IPV4_DESTINATION_OFFSET+IPV4_DESTINATION_LENGTH)

#define UDP_SOURCE_PORT_LENGTH 2
#define UDP_DESTINATION_PORT_LENGTH 2
#define UDP_LENGTH_LENGTH 2
#define UDP_CHECKSUM_LENGTH 2

#define UDP_SOURCE_PORT_OFFSET (IPV4_PAYLOAD_OFFSET)
#define UDP_DESTINATION_PORT_OFFSET (UDP_SOURCE_PORT_OFFSET+UDP_SOURCE_PORT_LENGTH)
#define UDP_LENGTH_OFFSET (UDP_DESTINATION_PORT_OFFSET+UDP_DESTINATION_PORT_LENGTH)
#define UDP_CHECKSUM_OFFSET (UDP_LENGTH_OFFSET+UDP_LENGTH_LENGTH)
#define UDP_PAYLOAD_OFFSET (UDP_CHECKSUM_OFFSET+UDP_CHECKSUM_LENGTH)

#define IPV4_TTL_LIMIT 2
#define UDP_IPV4_PROTOCOL_NUMBER 0x11

#define PACKET_HANDLER_FUNCTION_PROTO( appName) uint8_t appName(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* payload, uint16_t len)
#define PACKET_HANDLER_FUNCTION( appName) uint8_t appName(uint8_t* src, uint16_t src_port, uint8_t* dst, uint16_t dst_port, uint8_t* payload, uint16_t len)

#ifdef  __cplusplus
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
                                        uint8_t* version_out,
                                        uint8_t* headerlength_out,
                                        uint8_t* dscp_out,
                                        uint8_t* ecn_out,
                                        uint16_t* totallength_out,
                                        uint16_t* fragmentidentification_out,
                                        uint8_t* flags_out,
                                        uint16_t* fragmentoffset_out,
                                        uint8_t* ttl_out,
                                        uint8_t* protocol_out,
                                        uint16_t* headerchecksum_out
                                        );
    

#ifdef  __cplusplus
}
#endif

#endif  /* UDP_H */

