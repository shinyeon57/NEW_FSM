#include "L2_FSMevent.h"
#include "L2_msg.h"
#include "L2_timer.h"
#include "L2_LLinterface.h"
#include "L3_LLinterface.h"
#include "protocol_parameters.h"

//FSM state -------------------------------------------------
#define L2STATE_IDLE              0
#define L2STATE_TX                1
#ifndef DISABLE_ARQ
#define L2STATE_ACK               2
#endif

//state variables
static uint8_t main_state = L2STATE_IDLE; //protocol state
static uint8_t prev_state = main_state;

//source/destination ID
static uint8_t myL2ID=1;
static uint8_t destL2ID=0;


//L2 PDU context/size
static uint8_t arqPdu[200];
static uint8_t sduIn[200];
static uint8_t pduSize;
static uint8_t sduLen;

//ARQ parameters -------------------------------------------------------------
static uint8_t seqNum = 0;     //ARQ sequence number
#ifndef DISABLE_ARQ
static uint8_t retxCnt = 0;    //ARQ retransmission counter
static uint8_t arqAck[5];      //ARQ ACK PDU
#endif

static uint8_t L2_validityCheck_ID(void)
{
    if (myL2ID == destL2ID)
    {
        debug("[WARNING] myID and destination ID is the same! my:%i, dest:%i\n", myL2ID, destL2ID);
        return 1;
    }

    return 0;
}

void L2_configDestId(uint8_t destId)
{
    destL2ID = destId;

    L2_validityCheck_ID();
}

int L2_LLI_handleConfigReq(uint8_t type, uint8_t value)
{
    #ifdef ENABLE_CHANGEIDCMD
    if (type == L2L3_CFGTYPE_SRCID)
    {
        myL2ID = value;
        L2_event_setEventFlag(L2_event_configSrcId);
    }
    else if (type == L2L3_CFGTYPE_DSTID)
    {
        L2_configDestId(value);
    }
    else
    {
        debug("[ERROR in L2 config : unrecognized config type : %i\n", type);
        return -1;
    }
    #endif
    return 0;
}

void L2_LLI_handleDataReq(uint8_t* sdu, uint8_t len)
{
    memcpy(sduIn, sdu, len);
    sduLen = len;

    L2_event_setEventFlag(L2_event_dataToSend);
}


void L2_initFSM(uint8_t myId, uint8_t destId)
{
    myL2ID = myId;
    destL2ID = destId;

    L2_event_clearAllEventFlag();

    L2_validityCheck_ID();

    L2_LLI_initLowLayer(myL2ID);
    L3_LLI_setDataReqFunc(L2_LLI_handleDataReq);
    L3_LLI_setConfigReqFunc(L2_LLI_handleConfigReq);
}

void L2_FSMrun(void)
{
    //debug message
    if (prev_state != main_state)
    {
        debug_if(DBGMSG_L2, "[L2] State transition from %i to %i\n", prev_state, main_state);
        prev_state = main_state;
    }

    //FSM should be implemented here! ---->>>>
    switch (main_state)
    {
        case L2STATE_IDLE: //IDLE state description
            
            if (L2_event_checkEventFlag(L2_event_dataRcvd)) //if data reception event happens
            {
                //Retrieving data info.
#ifndef DISABLE_ARQ
                uint8_t srcId = L2_LLI_getSrcId();
#endif
                uint8_t* dataPtr = L2_LLI_getRcvdDataPtr();
                uint8_t size = L2_LLI_getSize();

                //debug("\n -------------------------------------------------\nRCVD from %i : %s (length:%i, seq:%i, RSSI:%i, SNR:%i)\n -------------------------------------------------\n", 
                //            srcId, L2_msg_getWord(dataPtr), size, L2_msg_getSeq(dataPtr), L2_LLI_getRssi(), L2_LLI_getSnr());
                L3_LLI_dataInd(L2_msg_getWord(dataPtr), size-L2_MSG_OFFSET_DATA, L2_LLI_getSnr(), L2_LLI_getRssi());

#ifdef DISABLE_ARQ
                main_state = L2STATE_IDLE;
#else
                //ACK transmission
                L2_msg_encodeAck(arqAck, L2_msg_getSeq(dataPtr));
                L2_LLI_sendData(arqAck, L2_MSG_ACKSIZE, srcId);

                main_state = L2STATE_TX; //goto TX state
#endif
                L2_event_clearEventFlag(L2_event_dataRcvd);
            }
            else if (L2_event_checkEventFlag(L2_event_dataToSend)) //if data needs to be sent (keyboard input)
            {
                //msg header setting
                pduSize = L2_msg_encodeData(arqPdu, sduIn, seqNum, sduLen);
                L2_LLI_sendData(arqPdu, pduSize, destL2ID);

#ifndef DISABLE_ARQ
                //Setting ARQ parameter 
                seqNum = (seqNum + 1)%L2_MSSG_MAX_SEQNUM;
                retxCnt = 0;
#endif
                debug_if(DBGMSG_L2, "[L2] sending to %i (seq:%i)\n", destL2ID, (seqNum-1)%L2_MSSG_MAX_SEQNUM);

                main_state = L2STATE_TX;

                L2_event_clearEventFlag(L2_event_dataToSend);
            }
            else if (L2_event_checkEventFlag(L2_event_configSrcId))
            {
                debug_if(DBGMSG_L2, "[L2] configuring source ID now...\n");
                if (L2_LLI_configSrcId(myL2ID) != 0)
                {
                    debug("[L2][ERROR] Failed to config source ID.");
                }
                L2_event_clearEventFlag(L2_event_configSrcId);
            }
#ifndef DISABLE_ARQ
            //ignore events (arqEvent_dataTxDone, arqEvent_ackTxDone, arqEvent_ackRcvd, arqEvent_arqTimeout)
            else if (L2_event_checkEventFlag(L2_event_dataTxDone)) //if data needs to be sent (keyboard input)
            {
                debug_if(DBGMSG_L2, "[L2][WARNING] cannot happen in IDLE state (event %i)\n", L2_event_dataTxDone);
                L2_event_clearEventFlag(L2_event_dataTxDone);
            }
            else if (L2_event_checkEventFlag(L2_event_ackTxDone)) //if data needs to be sent (keyboard input)
            {
                debug_if(DBGMSG_L2, "[L2][WARNING] cannot happen in IDLE state (event %i)\n", L2_event_ackTxDone);
                L2_event_clearEventFlag(L2_event_ackTxDone);
            }
            else if (L2_event_checkEventFlag(L2_event_dataTxDone)) //if data needs to be sent (keyboard input)
            {
                debug_if(DBGMSG_L2, "[L2][WARNING] cannot happen in IDLE state (event %i)\n", L2_event_ackRcvd);
                L2_event_clearEventFlag(L2_event_ackRcvd);
            }
            else if (L2_event_checkEventFlag(L2_event_arqTimeout)) //if data needs to be sent (keyboard input)
            {
                debug_if(DBGMSG_L2, "[WARNING] cannot happen in IDLE state (event %i)\n", L2_event_arqTimeout);
                L2_event_clearEventFlag(L2_event_arqTimeout);
            }   
#endif
            break;

        case L2STATE_TX: //TX state description

#ifndef DISABLE_ARQ
            if (L2_event_checkEventFlag(L2_event_ackTxDone)) //data TX finished
            {
                if (L2_timer_getTimerStatus() == 1 ||
                    L2_event_checkEventFlag(L2_event_arqTimeout))
                {
                    main_state = L2STATE_ACK;
                }
                else
                {
                    main_state = L2STATE_IDLE;
                }

                L2_event_clearEventFlag(L2_event_ackTxDone);
            }
            else 
#endif
            {
                if (L2_event_checkEventFlag(L2_event_dataTxDone)) //data TX finished
                {
#ifdef DISABLE_ARQ
                    main_state = L2STATE_IDLE;
#else
                    main_state = L2STATE_ACK;
                    L2_timer_startTimer(); //start ARQ timer for retransmission
#endif
                    L2_event_clearEventFlag(L2_event_dataTxDone);
                }
            }

            break;

#ifndef DISABLE_ARQ
        case L2STATE_ACK: //ACK state description

            if (L2_event_checkEventFlag(L2_event_ackRcvd)) //data TX finished
            {
                uint8_t* dataPtr = L2_LLI_getRcvdDataPtr();
                if ( L2_msg_getSeq(arqPdu) == L2_msg_getSeq(dataPtr) )
                {
                    debug_if(DBGMSG_L2, "[L2] ACK is correctly received! \n");
                    L2_timer_stopTimer();
                    main_state = L2STATE_IDLE;
                }
                else
                {
                    debug_if(DBGMSG_L2, "[L2]ACK seq number is weird! (expected : %i, received : %i\n", L2_msg_getSeq(arqPdu),L2_msg_getSeq(dataPtr));
                }

                L2_event_clearEventFlag(L2_event_ackRcvd);
            }
            else if (L2_event_checkEventFlag(L2_event_arqTimeout)) //data TX finished
            {
                if (retxCnt >= L2_ARQ_MAXRETRANSMISSION)
                {
                    debug("[L2][WARNING] Failed to send data %i, max retx cnt reached! \n", L2_msg_getSeq(arqPdu));
                    main_state = L2STATE_IDLE;
                    //arqPdu clear
                    //retxCnt clear
                }
                else //retx < max, then goto TX for retransmission
                {
                    debug_if(DBGMSG_L2, "[L2] timeout! retransmit\n");
                    L2_LLI_sendData(arqPdu, pduSize, destL2ID);
                    //Setting ARQ parameter 
                    retxCnt += 1;
                    main_state = L2STATE_TX;
                }

                L2_event_clearEventFlag(L2_event_arqTimeout);
            }
            else if (L2_event_checkEventFlag(L2_event_dataRcvd)) //data TX finished
            {
                //Retrieving data info.
                uint8_t srcId = L2_LLI_getSrcId();
                uint8_t* dataPtr = L2_LLI_getRcvdDataPtr();
                uint8_t size = L2_LLI_getSize();

                //debug("\n -------------------------------------------------\nRCVD from %i : %s (length:%i, seq:%i, RSSI:%i, SNR:%i)\n -------------------------------------------------\n", 
                //            srcId, L2_msg_getWord(dataPtr), size, L2_msg_getSeq(dataPtr), L2_LLI_getRssi(), L2_LLI_getSnr());
                L3_LLI_dataInd(L2_msg_getWord(dataPtr), size-L2_MSG_OFFSET_DATA, L2_LLI_getSnr(), L2_LLI_getRssi());
                            
                //ACK transmission
                L2_msg_encodeAck(arqAck, L2_msg_getSeq(dataPtr));
                L2_LLI_sendData(arqAck, L2_MSG_ACKSIZE, srcId);

                main_state = L2STATE_TX; //goto TX state

                L2_event_clearEventFlag(L2_event_dataRcvd);
            }
            else if (L2_event_checkEventFlag(L2_event_dataTxDone)) //data TX finished
            {
                debug_if(DBGMSG_L2, "[L2][WARNING] cannot happen in ACK state (event %i)\n", L2_event_dataTxDone);
                L2_event_clearEventFlag(L2_event_dataTxDone);
            }
            else if (L2_event_checkEventFlag(L2_event_ackTxDone)) //data TX finished
            {
                debug_if(DBGMSG_L2, "[L2][WARNING] cannot happen in ACK state (event %i)\n", L2_event_ackTxDone);
                L2_event_clearEventFlag(L2_event_ackTxDone);
            }

            break;
#endif
        default :
            break;
    }

}