
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "ethernet.h"

//TODO stop copying, just copy the pointers, you can build this stack just with 600 bytes
	static uint8_t local_eth_address[6]={0xFF, 0x00, 0x00, 0x00, 0x00, 0x01};
    static const uint8_t eth_broadcast_address[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    static const uint32_t eth_crc_table[] =
    {
        0x4DBDF21C, 0x500AE278, 0x76D3D2D4, 0x6B64C2B0,
        0x3B61B38C, 0x26D6A3E8, 0x000F9344, 0x1DB88320,
        0xA005713C, 0xBDB26158, 0x9B6B51F4, 0x86DC4190,
        0xD6D930AC, 0xCB6E20C8, 0xEDB71064, 0xF0000000
    };

    static uint16_t eth_compute_crc(uint8_t* buf, uint16_t len)
    {
    	uint16_t n;
    	uint32_t crc=0;
        for (n=0; n<len; n++)
        {
            crc = (crc >> 4) ^ eth_crc_table[(crc ^ (buf[n] >> 0)) & 0x0F];  /* lower nibble */
            crc = (crc >> 4) ^ eth_crc_table[(crc ^ (buf[n] >> 4)) & 0x0F];  /* upper nibble */
        }
        return crc;
    }


    void eth_initialize_network(uint8_t* myEth)
    {
        uint32_t i;
        for(i=0; i<6; i++)
            local_eth_address[i] = myEth[i];
    }

    uint8_t* eth_get_local_address(uint8_t* eth_out)
    {
        uint8_t i;

        if(eth_out!=NULL)
        {
            for(i=0; i<6; i++)
                eth_out[i] = local_eth_address[i];
        }
        return local_eth_address;
    }

    uint8_t* eth_get_broadcast_address(uint8_t* eth_out)
    {
        uint8_t i;

        if(eth_out!=NULL)
        {
            for(i=0; i<6; i++)
                eth_out[i] = eth_broadcast_address[i];
        }
        return eth_broadcast_address;
    }
    /*
     * preparePacket()
     * prepares a packet with source address, target destination address and payload and puts it in packet_out
     * also computes the checksum and also puts it into the packet
     * on successful encapsulation function returns the length of the packet
     * else returns zero
     *
     * note: since our application won't and can't use csma/cd this implementation ignores the padding which is used for csma/cd
     * therefore this packet may contain payloads with lengths lower then 40 bytes
     */
    uint32_t eth_create_packet(uint8_t* src_in, uint8_t* dst_in, uint8_t* payload_in, uint16_t payload_length, uint8_t* packet_out)
    {
        uint32_t crc=0;
        uint16_t len=0;
        uint16_t eth_len=0;

        //check for input errors
        if(payload_length > ETH_MAX_PAYLOAD_LENGTH || packet_out==NULL)
            return 0;

        //destination address
        memcpy(packet_out+ETH_DESTINATION_OFFSET, dst_in, ETH_DESTINATION_LENGTH);
        len+=ETH_DESTINATION_LENGTH;

        //source address
        memcpy(packet_out+ETH_SOURCE_OFFSET, src_in, ETH_SOURCE_LENGTH);
        len+=ETH_SOURCE_LENGTH;

        //packet length
        eth_len=ETH_DESTINATION_LENGTH+ETH_SOURCE_LENGTH+ETH_LENGTH_LENGTH+payload_length+ETH_FCS_LENGTH;
        packet_out[len]=eth_len>>8 & 0xFF;
        packet_out[len+1]=eth_len & 0xFF;
        len+=ETH_LENGTH_LENGTH;

        //payload
        memcpy(packet_out+ETH_PAYLOAD_OFFSET, payload_in, payload_length);
        len+=payload_length;

        //fcs (crc32)
        crc=eth_compute_crc(packet_out, len);

        packet_out[len]=crc>>24 & 0xFF;
        packet_out[len+1]=crc>>16 & 0xFF;
        packet_out[len+2]=crc>>8 & 0xFF;
        packet_out[len+3]=crc & 0xFF;
        len+=ETH_FCS_LENGTH;

        return len;
    }
    /*
     * checkDestination()
     * checks the destination of the packet_in with my_dst
     * if packet_dst is not null pointer writes the packet's destination to packet_dst
     * returns zero if the addresses match
     */
    uint8_t eth_check_destination(uint8_t* my_dst, uint8_t* packet_dst, uint8_t* packet_in)
    {
        uint8_t result;
        //check for address match
        result=memcmp(my_dst, packet_in+ETH_DESTINATION_OFFSET, ETH_DESTINATION_LENGTH);
        if(result)
        {
            result=memcmp(eth_broadcast_address, packet_in+ETH_DESTINATION_OFFSET, ETH_DESTINATION_LENGTH);
        }

        //copy the destination address in the packet
        if(packet_dst!=NULL)
        	memcpy(packet_dst, packet_in+ETH_DESTINATION_OFFSET, ETH_DESTINATION_LENGTH);
        return result;
    }
    /*
     * openPacket()
     * opens the packet_in and writes source address to src_out, destination address to dst_out
     * writes the payload to payload_out
     * before writing anything it first checks the checksum, if the checksum doesn't match,
     * null is written to all pointers and function returns 0
     * on a successful opening function returns the length of the packet
     */
    uint16_t eth_open_packet(uint8_t* src_out, uint8_t* dst_out, uint8_t* payload_out, uint8_t* packet_in, uint16_t packet_length)
    {
        uint16_t len=0;
        uint16_t eth_len=0;
        uint32_t crc=0, packet_crc=0;
        //copy destination address
        if(dst_out!=NULL)
        	memcpy(dst_out, packet_in+ETH_DESTINATION_OFFSET, ETH_DESTINATION_LENGTH);

        //copy source address
        if(src_out!=NULL)
        	memcpy(src_out, packet_in+ETH_SOURCE_OFFSET, ETH_SOURCE_LENGTH);

        //copy ethernet packet length
        eth_len=packet_in[ETH_LENGTH_OFFSET] & 0xFF;
        eth_len=eth_len<<8;
        eth_len|=packet_in[ETH_LENGTH_OFFSET+1] & 0xFF;

        //printf("packet_length=%d\neth_len=%d\n",packet_length, eth_len);
        //calculate payload length
        if(packet_length!=eth_len)
        	return 0;

        len=eth_len-ETH_DESTINATION_LENGTH-ETH_SOURCE_LENGTH-ETH_LENGTH_LENGTH-ETH_FCS_LENGTH;

        //copy the crc came with packet
        packet_crc=packet_in[ETH_PAYLOAD_OFFSET+len] & 0xFF;
        packet_crc=packet_crc<<8;
        packet_crc|=packet_in[ETH_PAYLOAD_OFFSET+len+1] & 0xFF;
        packet_crc=packet_crc<<8;
        packet_crc|=packet_in[ETH_PAYLOAD_OFFSET+len+2] & 0xFF;
        packet_crc=packet_crc<<8;
        packet_crc|=packet_in[ETH_PAYLOAD_OFFSET+len+3] & 0xFF;

        //calculate fcs (crc32)
        crc=eth_compute_crc(packet_in, eth_len-ETH_FCS_LENGTH);

        //check for match
        if(packet_crc != crc)
            return 0;


        if(payload_out!=NULL)
        {
            memcpy(payload_out, packet_in+ETH_PAYLOAD_OFFSET, len);
            payload_out[len]=0;
        }

        return len;
    }
