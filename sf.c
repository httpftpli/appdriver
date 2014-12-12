#include "sf.h"
#include "pf_can.h"
#include "module.h"
#include "pf_timertick.h"
#include "atomic.h"
#include <string.h>
#include "mmath.h"


//sifu
#define CAN_WP_FUNCODE_SF_ALARM                 0x14
#define CAN_WP_FUNCODE_SF_READPARAM             0x10
#define CAN_WP_FUNCODE_SF_SETPARAM              0x11
#define CAN_WP_FUNCODE_SF_READMONITPARAM        0x12
#define CAN_WP_FUNCODE_SF_CHECKZERO             0x15
#define CAN_WP_FUNCODE_SF_CHECKZEROFINISH       0x16
#define CAN_WP_FUNCODE_SF_SPEENRUN              0x20
#define CAN_WP_FUNCODE_SF_PULSERUN              0x21
#define CAN_WP_FUNCODE_SF_STOP                  0x22
#define CAN_WP_FUNCODE_SF_READSTAT              0x23


static  atomic g_flagSfHeartBeat,g_flagSfReadParm,
g_flagSfSetParm,g_flagSfReadMonitParm,g_flagSfZeroCheck,
g_flagSfZeroCheckFinish,g_flagSfReadStat,g_flagSfAlarm;


static unsigned char g_sfReadStat,g_sfAlarmCode;

static unsigned short g_sfParam[30];
static unsigned short g_sfMonitParam[30];

static unsigned char paramipacknext = 0;
static unsigned char monitoripacknext = 0;



bool sfReadParam(unsigned char id, unsigned short *param, unsigned int len, unsigned int timeout) {
    paramipacknext = 0;
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_SF_READPARAM;
    frame.desid = SIFU_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&g_flagSfReadParm)) {
            memcpy(param, g_sfParam, len);
            return true;
        }
    }
    return false;
}


bool sfSetParam(unsigned char id, unsigned short *param, unsigned int len, unsigned int timeout) {
    monitoripacknext = 0;
    DEFINE_CAN_WP_FRAME(frame);
    unsigned int packnum = DIVUP(len, 3);
    frame.funcode = CAN_WP_FUNCODE_SF_SETPARAM;
    frame.desid = SIFU_ID(id);
    for (unsigned int i = 0; i < packnum - 1; i++) {
        frame.dlc = 8;
        frame.data[0] = i << 8 | packnum;
        frame.data[0] |= param[3 * i] << 16;
        frame.data[1] |= param[3 * i + 1] | param[3 * i + 2] << 16;
        CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    }
    //process reset
    unsigned int rest = packnum % 3;
    if (packnum != 0) {
        frame.dlc = 2 + rest * 2;
        frame.data[0] = (packnum - 1) << 8 | packnum;
        unsigned  short *p = (unsigned short *)(&frame.data[0]) + 1;
        for (int i = 0; i < rest; i++) {
            p[i] = param[packnum * 3 + i];
        }
        CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    }
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&g_flagSfSetParm)) {
            return true;
        }
    }
    return false;
}


bool sfReadMonitParam(unsigned char id, unsigned short *param, unsigned int len, unsigned int timeout) {
    monitoripacknext = 0;
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_SF_READMONITPARAM;
    frame.desid = SIFU_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&g_flagSfReadMonitParm)) {
            memcpy(param, g_sfMonitParam, len);
            return true;
        }
    }
    return false;
}



bool sfCheckZero(unsigned char id, unsigned int timeout) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_SF_CHECKZERO;
    frame.desid = SIFU_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&g_flagSfZeroCheck)) {
            return true;
        }
    }
    return false;
}


void sfSpeedRun(unsigned char id, bool wise_clockwise0, unsigned short speed) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_SF_SPEENRUN;
    frame.desid = SIFU_ID(id);
    frame.dlc = 3;
    frame.data[0] = wise_clockwise0 | speed << 8;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void sfPulseRun(unsigned char id, bool wise_clockwise0, unsigned int pulse) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_SF_PULSERUN;
    frame.desid = SIFU_ID(id);
    frame.dlc = 5;
    frame.data[0] = wise_clockwise0 | pulse << 8;
    frame.data[1] = pulse >> 24;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void sfStop(unsigned char id, bool magnet) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_SF_STOP;
    frame.desid = SIFU_ID(id);
    frame.dlc = 1;
    frame.data[0] = magnet;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}


bool  sfReadStat(unsigned char id, unsigned char *stat,unsigned int timeout) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_SF_READSTAT;
    frame.desid = SIFU_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&g_flagSfReadStat)) {
            *stat =  g_sfReadStat;
            return true;
        }
    }
    return  false ;
}


#define IDLE  0
#define RCV   1

void canSfRcv(CAN_WP *frame) {
    if (frame->flag != 1) return;
    if (frame->srcid & 0xf0 != CAN_WP_DEV_TYPE_SIFU) {
        return; //device id is not sifu
    }
    unsigned int funcode = frame->funcode;
    switch (funcode) {
    case CAN_WP_FUNCODE_SF_ALARM:
        g_sfAlarmCode = (unsigned char)frame->data[0];
        atomicSet(g_flagSfAlarm);
        break;
    case CAN_WP_FUNCODE_SF_READPARAM:
        {
            unsigned char parampacknum = (unsigned char)frame->data[0];
            unsigned char paramipack = (unsigned char)(frame->data[0] >> 8);
            if ((paramipack >= parampacknum) || (parampacknum > lenthof(g_sfParam)/3)) {
                paramipacknext = 0;
                break;
            }
            if (frame->dlc < 4 || frame->dlc / 2 != 0) {
                paramipacknext = 0;
                break;
            }
            if (frame->dlc!=8 && paramipack!=parampacknum-1) {
                paramipacknext = 0;
                break;
            }
            if (paramipack!=paramipacknext++) {
                paramipacknext = 0;
                break;
            }
            for (int i=0;i<(frame->dlc-2)/2;i++) {
                g_sfParam[paramipack * 3+i] = ((unsigned short *)(&frame->data[0]))[1+i] ;
            }
            if (paramipack+1==parampacknum) {
                paramipacknext = 0;
                atomicSet(&g_flagSfReadParm);
            }
        }
        break;
    case CAN_WP_FUNCODE_SF_SETPARAM:
        atomicSet(g_flagSfSetParm);
        break;
    case CAN_WP_FUNCODE_SF_READMONITPARAM:
        {
            unsigned char parampacknum = (unsigned char)frame->data[0];
            unsigned char paramipack = (unsigned char)(frame->data[0] >> 8);
            if ((paramipack >= parampacknum) || (parampacknum > lenthof(g_sfParam)/3)) {
                monitoripacknext = 0;
                break;
            }
            if (frame->dlc < 4 || frame->dlc / 2 != 0) {
                monitoripacknext = 0;
                break;
            }
            if (frame->dlc!=8 && paramipack!=parampacknum-1) {
                monitoripacknext = 0;
                break;
            }
            if (paramipack!=monitoripacknext++) {
                monitoripacknext = 0;
                break;
            }
            for (int i=0;i<(frame->dlc-2)/2;i++) {
                g_sfMonitParam[paramipack * 3+i] = ((unsigned short *)(&frame->data[0]))[1+i] ;
            }
            if (paramipack+1==parampacknum) {
                monitoripacknext = 0;
                atomicSet(&g_flagSfReadMonitParm);
            }
        }
        break;
    case CAN_WP_FUNCODE_SF_CHECKZERO:
        atomicSet(g_flagSfZeroCheck);
        break;
    case CAN_WP_FUNCODE_SF_CHECKZEROFINISH:

        atomicSet(g_flagSfZeroCheckFinish);
        break;
    case CAN_WP_FUNCODE_SF_READSTAT:
        g_sfReadStat = (unsigned char )frame->data[0];
        atomicSet(&g_flagSfReadStat);
        break;
    default:
        break;
    }
}
