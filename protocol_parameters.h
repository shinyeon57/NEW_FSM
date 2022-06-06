#define DBGMSG_L2                       1 //debug print control
#define DBGMSG_L3                       1 //debug print control

#define L3_MAXDATASIZE                  26


#define L2_ARQ_MAXRETRANSMISSION        10
#define L2_ARQ_MAXWAITTIME              5
#define L2_ARQ_MINWAITTIME              2



#ifdef ENABLE_CHANGEIDCMD
#define L2L3_CFGTYPE_SRCID              0
#define L2L3_CFGTYPE_DSTID              1
#endif