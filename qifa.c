
#include "can_wp.h"
#include "pf_can.h"
#include "qifa.h"
#include "module.h"
#include "debug.h"
#include <string.h>

//qifa
#define CAN_WP_FUNCODE_QF_POWERSAVE     0x21
#define CAN_WP_FUNCODE_QF_EN            0x22
#define CAN_WP_FUNCODE_QF_RESET         0x23
#define CAN_WP_FUNCODE_QF_SETDUTY       0x24
#define CAN_WP_FUNCODE_QF_WRITE         0x25
#define CAN_WP_FUNCODE_QF_READOHM       0x2c
#define CAN_WP_FUNCODE_QF_SETALARMCODE  0x26
#define CAN_WP_FUNCODE_QF_SETALARMMASK  0x27
#define CAN_WP_FUNCODE_QF_READALARM     0x28
#define CAN_WP_FUNCODE_QF_READ          0x29
#define CAN_WP_FUNCODE_QF_ALARM         0x2b




#define QIFA_ID(ID)    (CAN_WP_DEV_TYPE_QIFA|ID)


void wpQfPowrSave(unsigned char  id, bool on){
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_POWERSAVE;
    frame.desid = QIFA_ID(id) ;
    frame.data[0] = (unsigned char)on;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}

void wpQfEn(unsigned char  id, bool on){
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_EN;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)on ;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}

void wpQfRst(unsigned char  id,bool rst){
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_RESET;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char) (!!rst);
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}

void wpQfSetDuty(unsigned char  id, unsigned char duty){
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_SETDUTY;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)duty;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}



void wpQfWrite(unsigned char id, unsigned char *val,unsigned int count ){
    ASSERT(count<9);
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_WRITE;
    frame.desid = QIFA_ID(id);
    memcpy(frame.data,val,count);
    frame.dlc = count;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}


void wpQfReadOhm(unsigned char id, unsigned char qifaId ){
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_READOHM;
    frame.desid = QIFA_ID(id);
    frame.data[0] = qifaId | (1<<8);
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}

void wpQfSetAlarmCode(unsigned char id,unsigned char code){
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_SETALARMCODE;
    frame.desid = QIFA_ID(id);
    frame.data[0] = code;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}

void wpQfSetAlarmMask(unsigned char id,unsigned char mask){
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_SETALARMMASK;
    frame.desid = QIFA_ID(id);
    frame.data[0] = mask;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}


void wpQfReadAlarm(unsigned char id){
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_READALARM;
    frame.desid = QIFA_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}

void wpQfRead(unsigned char id){
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_READ;
    frame.desid = QIFA_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0,(CAN_FRAME *)&frame);
}

static unsigned short qfOhm[4][16];

void canQfRcv(CAN_WP *frame){
    if (frame->flag != 1 ) return;
    if (frame->srcid & 0xf0 != CAN_WP_DEV_TYPE_QIFA) {
        return ;//device id is not qifa
    }
    unsigned int funcode = frame->funcode;
    switch (funcode) {
    case CAN_WP_FUNCODE_QF_READOHM:
        {
            unsigned char qfban = (frame->srcid & 0xf)-1;
            unsigned char qf = (unsigned char)(frame->data[0])-1;

            if ((qfban>3)||(qf>16)) return;
            qfOhm[qfban][qf] = (unsigned short) (frame->data[0]>>8);
        }
        break ;
    case CAN_WP_FUNCODE_QF_READALARM:
        break ;
    case CAN_WP_FUNCODE_QF_READ:
        break;
    case CAN_WP_FUNCODE_QF_ALARM:
        break;
    default:
        break;
    }
}
