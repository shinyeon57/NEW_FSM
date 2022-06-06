void L2_LLI_initLowLayer(uint8_t srcId);
void L2_LLI_sendData(uint8_t* msg, uint8_t size, uint8_t dest);
uint8_t L2_LLI_getSrcId();
uint8_t* L2_LLI_getRcvdDataPtr();
uint8_t L2_LLI_getSize();
int16_t L2_LLI_getRssi(void);
int8_t L2_LLI_getSnr(void);
int L2_LLI_configSrcId(uint8_t id);