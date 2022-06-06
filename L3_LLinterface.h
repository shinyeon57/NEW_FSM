extern void (*L3_LLI_dataReqFunc)(uint8_t* msg, uint8_t size);
extern int (*L3_LLI_configReqFunc)(uint8_t type, uint8_t value);
//syntax : L3_LLI_configReqFunc(L2L3_CFGTYPE_SRCID, 2);

void L3_LLI_dataInd(uint8_t* dataPtr, uint8_t size, int8_t snr, int16_t rssi);
uint8_t* L3_LLI_getMsgPtr();
uint8_t L3_LLI_getSize();
void L3_LLI_setDataReqFunc(void (*funcPtr)(uint8_t*, uint8_t));
void L3_LLI_setConfigReqFunc(int (*funcPtr)(uint8_t, uint8_t));