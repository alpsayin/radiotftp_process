/* 
 * File:   tftp.h
 * Author: alpsayin
 *
 * Created on April 1, 2012, 3:31 PM
 */

#ifndef TFTP_H
#define	TFTP_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdint.h>
    
#include "udp_ip.h"
#include "timers.h"

#define TFTP_STATUS_IDLE             	0
#define TFTP_STATUS_SENDING             1
#define TFTP_STATUS_RECEIVING           2

#define TFTP_OPCODE_RRQ     	0x0001
#define TFTP_OPCODE_WRQ     	0x0002
#define TFTP_OPCODE_DATA    	0x0003
#define TFTP_OPCODE_ACK     	0x0004
#define TFTP_OPCODE_ERROR   	0x0005
#define TFTP_OPCODE_WRQ_SINGLE	0x0006

#define TFTP_ERROR_SEE_MESSAGE          0x0000
#define TFTP_ERROR_FILE_NOT_FOUND       0x0001
#define TFTP_ERROR_ACCESS_VIOLATION    	0x0002
#define TFTP_ERROR_DISK_FULL            0x0003
#define TFTP_ERROR_ILLEGAL_OPERATION    0x0004
#define TFTP_ERROR_UKNOWN_TID           0x0005
#define TFTP_ERROR_FILE_EXISTS          0x0006
#define TFTP_ERROR_NO_USER              0x0007

#define TFTP_MAX_BLOCK_SIZE		512
#define TFT_DATA_HEADER_SIZE 	4

#define TFTP_SINGLE_BLOCK_WAIT_TIME 1
#define TFTP_ACK_TIMEOUT_MIN 		1
#define TFTP_ACK_TIMEOUT_MAX 		2
#define TFTP_READ_TIMEOUT 			5
#define TFTP_COMPLETE_TIMEOUT 		5
#define TFTP_MAX_TIMEOUTS   		10

#define TFTP_DEFAULT_FILENAME "sensors.dat"

    typedef struct
    {
        uint8_t src[6];
        uint8_t dst[6];
        uint16_t src_port;
        uint16_t dst_port;
        uint16_t opcode;
        uint8_t payload[UDP_MAX_PAYLOAD_LENGTH];
        uint16_t payloadLength;
        uint16_t blockNumber;
        uint8_t append;
    } message_t;

    PACKET_HANDLER_FUNCTION_PROTO(tftp_transfer);

    TIMER_HANDLER_FUNCTION_PROTO(tftp_timer_handler);

    uint8_t tftp_initialize(dataQueuerfptr_t dataQueuer);

    uint8_t tftp_sendSingleBlockData(uint8_t* dst_ip, uint8_t* data_ptr, uint16_t data_len, uint8_t* remote_filename);
    uint8_t tftp_sendRequest(uint8_t opcode, uint8_t* dst_ip, uint8_t* local_databuffer, uint16_t local_databuffer_len, uint8_t* remote_filename, uint8_t remote_filename_len, uint8_t append);
    uint8_t tftp_sendData(uint8_t* dst_ip, uint8_t blockNum);
    uint8_t tftp_sendError(uint8_t type, uint8_t* dst_ip, uint16_t dst_prt, uint8_t* additionalInfo, uint8_t infoLen);
    uint8_t tftp_sendAck(uint8_t* dst_ip, uint8_t blockNum);

    uint8_t tftp_getRandomRetransmissionTime(void);

    uint8_t tftp_getStatus(void);
    void tftp_setStatus(uint8_t newStatus);

    uint16_t tftp_transfer_src_port(void);
    uint16_t tftp_transfer_dst_port(void);

#ifdef	__cplusplus
}
#endif

#endif	/* TFTP_H */

