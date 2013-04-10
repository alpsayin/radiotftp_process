#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "udp_ip.h"
#include "tftp.h"
#include "timers.h"
#include "radiotftp.h"
#include "util.h"
#include "avr_util.h"

static uint8_t* data_buffer;
static uint16_t fileLen;
static uint16_t buffer_pos=0;
static message_t lastMessage;
static uint8_t status=TFTP_STATUS_IDLE;
static uint8_t timeouts=0;
static uint8_t isRequestOwner=0;
static uint16_t blockNumber=0;
static uint16_t ackNumber=0;
static uint16_t tftp_dst_port=70;
static uint16_t tftp_src_port=71;
static dataQueuerfptr_t mainDataQueuer;

uint8_t tftp_initialize(dataQueuerfptr_t dataQueuer)
{
    mainDataQueuer=dataQueuer;
    tftp_setStatus(TFTP_STATUS_IDLE);
    //for testing
    //timers_create_timer("TFTP Timer", &tftp_timer, 3, 0);
    return 0;
}
uint8_t tftp_getStatus(void)
{
	return status;
}
void tftp_setStatus(uint8_t newStatus)
{
    status=newStatus;
}
uint16_t tftp_transfer_src_port(void)
{
    return tftp_src_port;
}
uint16_t tftp_transfer_dst_port(void)
{
    return tftp_dst_port;
}
uint8_t tftp_getRandomRetransmissionTime(void)
{
    uint8_t time = TFTP_ACK_TIMEOUT_MIN + (TFTP_ACK_TIMEOUT_MAX-TFTP_ACK_TIMEOUT_MIN+1)*(((float)rand())/((float)RAND_MAX));
    //PRINTF_D("time=%d\n", time);
    return time;
}
uint8_t tftp_sendSingleBlockData(uint8_t* dst_ip, uint8_t* data_ptr, uint16_t data_len, uint8_t* remote_filename)
{
	uint8_t filenameCheck=0;

	if(data_len>450)
	{
		printf("input data too large, %d\n", data_len);
		return (-1);
	}
	printf("destination = ");
	print_addr_dec(dst_ip);
//	printf("local-data-length = %d\n", data_len);
	filenameCheck |= (data_ptr==NULL);
	filenameCheck = filenameCheck<<1;
//	printf("remote-filename = '%s'\n", remote_filename);
	filenameCheck |= (remote_filename==NULL || remote_filename[0]==0x00 );
//	printf("filename-check = 0x%02x\n", filenameCheck);
	if(filenameCheck & 0x02)
	{
		printf("empty local data ptr\n");
		return (-2);
	}
	if(filenameCheck & 0x01)
	{
		printf("empty remote filename, using default filename = "TFTP_DEFAULT_FILENAME"\n");
		remote_filename=TFTP_DEFAULT_FILENAME;
	}
	if(filenameCheck == 0x03)
	{
		printf("empty input filenames");
		return (-3);
	}
    lastMessage.payloadLength=0;
    //put opcode in
    lastMessage.opcode=TFTP_OPCODE_WRQ_SINGLE;
    //put source ip in
    udp_get_localhost_ip(lastMessage.src);
    //put destination ip in
    memcpy(lastMessage.dst, dst_ip, 6);
    //select destination port
    lastMessage.dst_port=69;
    //select a random src port
    do
    {
        lastMessage.src_port= 65535*(((float)rand())/((float)RAND_MAX));
    }while(lastMessage.src_port==69 || lastMessage.src_port==0);
    //assign the tft_src_port
    tftp_src_port=lastMessage.src_port;
    printf("tftp src port = %d\n", tftp_src_port);
    //create the payload
    lastMessage.payload[lastMessage.payloadLength++] = 0x00;
    lastMessage.payload[lastMessage.payloadLength++] = TFTP_OPCODE_WRQ_SINGLE;
    memcpy(lastMessage.payload+lastMessage.payloadLength, remote_filename, strnlen(remote_filename, 16));
    lastMessage.payloadLength+=strnlen(remote_filename, 16);
    printf("remote_filename = '%s'\n", remote_filename);
    memcpy(lastMessage.payload+lastMessage.payloadLength, "\0netascii\0", 10);
    lastMessage.payloadLength+=10;
    lastMessage.append=0;
    memcpy(&(lastMessage.payload[lastMessage.payloadLength]), data_ptr, data_len);
    lastMessage.payloadLength+=data_len;

    //put the block number in
    lastMessage.blockNumber = blockNumber=0;

    //set a timer to exit after a certain amount of time
	timers_create_timer(TFTP_SINGLE_BLOCK_WAIT_TIME, 128);


    //reset acks
    ackNumber=-1;
    blockNumber=0;
    isRequestOwner=1;
    timeouts=0;
    return mainDataQueuer(udp_get_localhost_ip(NULL), lastMessage.src_port, lastMessage.dst, lastMessage.dst_port, lastMessage.payload, lastMessage.payloadLength);
}
uint8_t tftp_sendRequest(uint8_t opcode, uint8_t* dst_ip, uint8_t* local_databuffer, uint16_t local_databuffer_len, uint8_t* remote_filename, uint8_t remote_filename_len, uint8_t append)
{
	uint8_t filenameCheck=0;
	PRINTF_D("destination = ");
	print_addr_dec(dst_ip);
	filenameCheck |= (local_databuffer==NULL);
	filenameCheck = filenameCheck<<1;
	//PRINTF_D("remote-filename = '%s' -> %d\n", remote_filename, remote_filename_len);
	filenameCheck |= (remote_filename==NULL || remote_filename_len==0 || remote_filename[0]==0x00 );
	//PRINTF_D("filename-check = 0x%02x\n", filenameCheck);
	if(filenameCheck & 0x02)
	{
		PRINTF_D("null pointer, exiting\n");
		return -1;
	}
	if(filenameCheck & 0x01)
	{
		PRINTF_D("empty remote filename, using default 'sensors.dat'\n");
		remote_filename="sensors.dat";
		remote_filename_len=strlen("sensors.dat");
	}
	if(filenameCheck == 0x03)
	{
		return -1;
	}
    if(opcode == TFTP_OPCODE_RRQ)
    {
    	printf("radiotftp_process does not receive files\n");
    }
    else if(opcode == TFTP_OPCODE_WRQ)
    {
        tftp_setStatus(TFTP_STATUS_SENDING);
        data_buffer = local_databuffer;
        if(data_buffer == NULL)
        {
            PRINTF_D("error opening file for read");
            return (-13);
        }
        fileLen=local_databuffer_len;
    }
    else
    {
        return -10;
    }
    lastMessage.payloadLength=0;
    //put opcode in
    lastMessage.opcode=opcode;
    //put source ip in
    udp_get_localhost_ip(lastMessage.src);
    //put destination ip in
    memcpy(lastMessage.dst, dst_ip, 6);
    //select destination port
    lastMessage.dst_port=69;
    //select a random src port
    do
    {
        lastMessage.src_port= 65535*(((float)rand())/((float)RAND_MAX));
    }while(lastMessage.src_port==69 || lastMessage.src_port==0);
    //assign the tft_src_port
    tftp_src_port=lastMessage.src_port;
    PRINTF_D("tftp src port = %d\n", tftp_src_port);
    //create the payload
    lastMessage.payload[lastMessage.payloadLength++] = 0x00;
    lastMessage.payload[lastMessage.payloadLength++] = opcode;
    memcpy(lastMessage.payload+lastMessage.payloadLength, remote_filename, remote_filename_len);
    lastMessage.payloadLength+=remote_filename_len;
    PRINTF_D("remote_filename = '%s'\n", remote_filename);
    memcpy(lastMessage.payload+lastMessage.payloadLength, "\0netascii\0", 10);
    lastMessage.payloadLength+=10;
    if(append)
    {
    	memcpy(lastMessage.payload+lastMessage.payloadLength, "append\0", 7);
    	lastMessage.payloadLength+=7;
    	lastMessage.append=1;
    }
    else
    {
    	lastMessage.payload[lastMessage.payloadLength]='\0';
    	lastMessage.payloadLength++;
    	lastMessage.append=0;
    }
    //put the block number in
    lastMessage.blockNumber = blockNumber=0;
    	//set up retransmit timer
    if(opcode==TFTP_OPCODE_WRQ || opcode==TFTP_OPCODE_RRQ)
    {
    	timers_create_timer(tftp_getRandomRetransmissionTime()+1, 128);
    }
    //reset acks
    ackNumber=-1;
    blockNumber=0;
    isRequestOwner=1;
    timeouts=0;
    return mainDataQueuer(udp_get_localhost_ip(NULL), lastMessage.src_port, lastMessage.dst, lastMessage.dst_port, lastMessage.payload, lastMessage.payloadLength);
}
uint8_t tftp_sendData(uint8_t* dst_ip, uint8_t blockNum)
{
    uint16_t curPos, writeLen;
//    PRINTF_D("tftp_sendData\n");
    lastMessage.payloadLength=0;
    //put opcode in
    lastMessage.opcode=TFTP_OPCODE_DATA;
    //put source ip in
    udp_get_localhost_ip(lastMessage.src);
    //put destination ip in
    memcpy(lastMessage.dst, dst_ip, 6);
    //select destination port
    lastMessage.dst_port=tftp_dst_port;
    //select a random src port
    lastMessage.src_port=tftp_src_port;
    //create the payload
    lastMessage.payload[lastMessage.payloadLength++] = 0x00;
    lastMessage.payload[lastMessage.payloadLength++] = lastMessage.opcode;
    lastMessage.payload[lastMessage.payloadLength++] = 0x00;
    lastMessage.payload[lastMessage.payloadLength++] = blockNum;
    //copy the data
#if 0
    curPos=buffer_pos; //this one was found to be faulty in case of retransmission
#else
    curPos=TFTP_MAX_BLOCK_SIZE*(blockNum-1);
#endif
//    PRINTF_D("tftp_sendData: before memcpy\n");
    if(fileLen-curPos < TFTP_MAX_BLOCK_SIZE)
    	writeLen=fileLen-curPos;
    else
    	writeLen=TFTP_MAX_BLOCK_SIZE;
    memcpy(lastMessage.payload+lastMessage.payloadLength, data_buffer+curPos, writeLen);
    lastMessage.payloadLength+=writeLen;
    curPos+=writeLen;

//    PRINTF_D("tftp_sendData: after memcpy\n");
    //put the block number in
    lastMessage.blockNumber = blockNum;
    //set up retransmit timer
    timers_create_timer(tftp_getRandomRetransmissionTime(), 128);
    PRINTF_D("sent data size = %u\n", lastMessage.payloadLength);
    return mainDataQueuer(udp_get_localhost_ip(NULL), lastMessage.src_port, lastMessage.dst, lastMessage.dst_port, lastMessage.payload, lastMessage.payloadLength);
}
uint8_t tftp_sendError(uint8_t type, uint8_t* dst_ip, uint16_t dst_prt, uint8_t* additionalInfo, uint8_t infoLen)
{
    lastMessage.payloadLength=0;
    //put opcode in
    lastMessage.opcode=TFTP_OPCODE_ERROR;
    //put source ip in
    udp_get_localhost_ip(lastMessage.src);
    //put destination ip in
    memcpy(lastMessage.dst, dst_ip, 6);
    //select destination port
    lastMessage.dst_port=dst_prt;
    //select a random src port
    lastMessage.src_port=tftp_src_port;
    //create the payload
    lastMessage.payload[lastMessage.payloadLength++] = 0x00;
    lastMessage.payload[lastMessage.payloadLength++] = lastMessage.opcode;
    lastMessage.payload[lastMessage.payloadLength++] = 0x00;
    lastMessage.payload[lastMessage.payloadLength++] = type;
    //copy the info
    if(additionalInfo!=NULL)
    {
    	memcpy(lastMessage.payload+lastMessage.payloadLength, additionalInfo, infoLen);
        lastMessage.payloadLength+=infoLen;
    }
    //set up retransmit timer
    //PRINTF_D("sent error size = %d\n", lastMessage.payloadLength);
    return mainDataQueuer(udp_get_localhost_ip(NULL), lastMessage.src_port, lastMessage.dst, lastMessage.dst_port, lastMessage.payload, lastMessage.payloadLength);
}
uint8_t tftp_sendAck(uint8_t* dst_ip, uint8_t blockNum)
{
    uint16_t i=0;
    uint8_t buffer[4];
    //create the payload
    buffer[i++]=0x00;
    buffer[i++]=TFTP_OPCODE_ACK;
    buffer[i++]= (blockNum>>8)&0xFF;
    buffer[i++]= (blockNum&0xFF);

    PRINTF_D("sent ack size = %d\n", i);
    return mainDataQueuer(udp_get_localhost_ip(NULL), tftp_src_port, dst_ip, tftp_dst_port, buffer, i);
}

PACKET_HANDLER_FUNCTION(tftp_transfer)
{
    uint8_t result=0;
    uint16_t opcode, block, error, i=0;

    //PRINTF_D("%s\n", payload);
    //read in the opcode
    opcode = payload[i++] & 0xFF;
    opcode <<= 8;
    opcode |= payload[i++] & 0xFF;

    //check the opcode
    if(status==TFTP_STATUS_SENDING)
    {
        if(opcode==TFTP_OPCODE_ACK)
        {
            if(timers_cancel_timer())
                PRINTF_D("couldnt cancel timer\n");
            block = payload[i++] & 0xFF;
            block <<= 8;
            block |= payload[i++] & 0xFF;
            ackNumber = block;
            PRINTF_D("tftp wrq ack #%d received\n", block);
            //prepare and send next packet

            tftp_dst_port=src_port;
            timeouts=0;
            result=tftp_sendData(src, ackNumber+1);
            if(result)
            {
                PRINTF_D("!!! couldn't send data #%d\n", ackNumber+1);
            }
            return 0;
        }
        else if(opcode==TFTP_OPCODE_ERROR)
        {
            //read in the error code
            error = payload[i++] & 0xFF;
            error <<= 8;
            error |= payload[i++] & 0xFF;
        	timers_cancel_timer();

            if(error==TFTP_ERROR_SEE_MESSAGE)
            {
                PRINTF_D("tftp error received -> %s\n", payload+i);
                if(!strncmp("TRANSMISSION COMPLETE", payload+i, strlen("TRANSMISSION COMPLETE")))
                {
                    if(isRequestOwner)
                    {
                    	return 0;
                    }
                    ackNumber=lastMessage.blockNumber;
                }
            }
            else
            {
                PRINTF_D("tftp error received %d\n", error);
            }
            //return to pending status
            status=TFTP_STATUS_IDLE;
            if(isRequestOwner)
            	return -16;
        }
        else
        {
            //silent discard
            return 0;
        }
    }
    else
    {
        //silent discard
        return 0;
    }

    return result;
}
TIMER_HANDLER_FUNCTION(tftp_timer_handler)
{
	//TODO something is really weird here with the control statements
	if(lastMessage.opcode==TFTP_OPCODE_WRQ_SINGLE)
	{
		printf("connection closed\n");
		if(isRequestOwner)
			return 0;
	}
	else
	{
		if(status==TFTP_STATUS_SENDING)
		{
			PRINTF_D("tftp_timer_handler\n");
			//if the last taken block number is less than the last transmitted ack number
			//or if we sent a write request and couldn't get an ack yet
			if( (lastMessage.blockNumber>ackNumber) || (lastMessage.opcode==TFTP_OPCODE_WRQ))
			{
				timeouts++;
				PRINTF_D("tftp ack timer timeout %d, timeouts=%d\n", ackNumber, timeouts);

				if(timeouts>=TFTP_MAX_TIMEOUTS)
				{
					tftp_setStatus(TFTP_STATUS_IDLE);
					blockNumber=0;
					ackNumber=0;
					PRINTF_D("connection canceled\n");
					if(isRequestOwner)
						return (-18);
					return 0;
				}

				//set up retransmit timer
				timers_create_timer(tftp_getRandomRetransmissionTime(), 128);
				//retransmit
				return mainDataQueuer(udp_get_localhost_ip(NULL), lastMessage.src_port, lastMessage.dst, lastMessage.dst_port, lastMessage.payload, lastMessage.payloadLength);
			}
		}
	}
	return 0;
}
