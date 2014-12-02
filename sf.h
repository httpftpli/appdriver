#ifndef ___SF__H__
#define ___SF__H__

#include "can_wp.h"
#include <stdbool.h>

#define SIFU_ID(ID)    (CAN_WP_DEV_TYPE_SIFU|ID)

extern bool sfHeartBeat(unsigned char id,unsigned int timeout);
extern bool sfReadParam(unsigned char id,unsigned short *param,unsigned int len,unsigned int timeout);
extern bool sfSetParam(unsigned char id,unsigned short *param,unsigned int len,unsigned int timeout);
extern bool sfReadMonitParam(unsigned char id,unsigned short *param,unsigned int len,unsigned int timeout);
extern bool sfCheckZero(unsigned char id,unsigned int timeout);
extern void sfSpeedRun(unsigned char id,bool wise_clockwise0,unsigned short speed);
extern void sfPulseRun(unsigned char id,bool wise_clockwise0,unsigned int pulse);
extern void sfStop(unsigned char id,bool magnet);
extern bool  sfReadStat(unsigned char id, unsigned char *stat,unsigned int timeout);
extern void canSfRcv(CAN_WP *frame);


#endif

