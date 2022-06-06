typedef enum L3_event
{
    L3_event_msgRcvd = 2,
    L3_event_dataToSend = 4,
    L3_event_MODEctrlRcvd = 5,              //제어정보IND
    L3_event_MODEctrlRcvd_CNN = 6,          //제어정보IND
    L3_event_MODEctrlRcvd_DND = 7,          //제어정보IND
    L3_event_MODEctrlRcvd_EXIT = 8,         //제어정보IND
    L3_event_DND_Timeout = 9,               //DND TIME OUT
    L3_event_CNN_Timeout = 10               //CNN TIME OUT
} L3_event_e;


void L3_event_setEventFlag(L3_event_e event);
void L3_event_clearEventFlag(L3_event_e event);
void L3_event_clearAllEventFlag(void);
int L3_event_checkEventFlag(L3_event_e event);