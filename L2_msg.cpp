#include "mbed.h"
#include "L2_msg.h"

int L2_msg_checkIfData(uint8_t* msg)
{
    return (msg[L2_MSG_OFFSET_TYPE] == L2_MSG_TYPE_DATA);
}

int L2_msg_checkIfAck(uint8_t* msg)
{
    return (msg[L2_MSG_OFFSET_TYPE] == L2_MSG_TYPE_ACK);
}

uint8_t L2_msg_encodeAck(uint8_t* msg_ack, uint8_t seq)
{
    msg_ack[L2_MSG_OFFSET_TYPE] = L2_MSG_TYPE_ACK;
    msg_ack[L2_MSG_OFFSET_SEQ] = seq;
    msg_ack[L2_MSG_OFFSET_DATA] = 1;

    return L2_MSG_ACKSIZE;
}

uint8_t L2_msg_encodeData(uint8_t* msg_data, uint8_t* data, int seq, int len)
{
    msg_data[L2_MSG_OFFSET_TYPE] = L2_MSG_TYPE_DATA;
    msg_data[L2_MSG_OFFSET_SEQ] = seq;
    memcpy(&msg_data[L2_MSG_OFFSET_DATA], data, len*sizeof(uint8_t));

    return len+L2_MSG_OFFSET_DATA;
}
                    

uint8_t L2_msg_getSeq(uint8_t* msg)
{
    return msg[L2_MSG_OFFSET_SEQ];
}

uint8_t* L2_msg_getWord(uint8_t* msg)
{
    return &msg[L2_MSG_OFFSET_DATA];
}