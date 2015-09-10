

#define QIFA_USE_MAILBOX   0

#include "can_wp.h"
#include "pf_can.h"
#include "qifa.h"
#include "module.h"
#include "debug.h"
#include <string.h>
#include "atomic.h"
#include "pf_timertick.h"
#include "mmath.h"
#include "ff.h"

#if QIFA_USE_MAILBOX==1
    #include "pf_mailbox.h"
#endif




//qifa
#define CAN_WP_FUNCODE_QF_POWERSAVE     			0x21		//设置气阀节能
#define CAN_WP_FUNCODE_QF_EN            				0x22		//气阀使能
#define CAN_WP_FUNCODE_QF_RESET         			0x23		//气阀复位
#define CAN_WP_FUNCODE_QF_SETDUTY       			0x24		//气阀占空比
#define CAN_WP_FUNCODE_QF_WRITE         			0x25		//气阀输出
#define CAN_WP_FUNCODE_QF_READOHM       			0x2c		//读取气阀阻值
#define CAN_WP_FUNCODE_QF_SETALARMCODE  		0x26		//设定气阀报警真值   报警信号在正常情况状态
#define CAN_WP_FUNCODE_QF_SETALARMMASK  		0x27		//报警屏蔽位  为0屏蔽 为1 需检测信号
#define CAN_WP_FUNCODE_QF_READALARM     			0x28		//读取报警信号
#define CAN_WP_FUNCODE_QF_READ          				0x29		//气阀状态读入
#define CAN_WP_FUNCODE_QF_ALARM         			0x2b		//气阀报警(气阀板主动上发)


#define QIFA_ID(ID)    (CAN_WP_ID(CAN_WP_DEV_TYPE_QIFA,ID))
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static atomic wp_read_qf_flag,wp_qf_resdata;
static unsigned short Sw_Qf_Status;
uint16 qifawarn[30];



void wpQfPowrSave(unsigned char id, bool on) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_POWERSAVE;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)on;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfEn(unsigned char id, bool on) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_EN;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)on;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfRst(unsigned char id, bool rst) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_RESET;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)(!!rst);
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfSetDuty(unsigned char id, unsigned char duty) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_SETDUTY;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)duty;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}



void wpQfWrite(unsigned char id, unsigned char *val, unsigned char count) {
    ASSERT(count < 9);
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_WRITE;
    frame.desid = QIFA_ID(id);
    memcpy(frame.data, val, count);
    frame.dlc = count;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}



bool wpQfReadOhm(unsigned char id, unsigned char qifaId, unsigned short timeout, unsigned short *Ohm) {
    QIFA *qifa = qifaSys.QiFa_Reg_Table[id - 1][qifaId];
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_READOHM;
    frame.desid = QIFA_ID(id);
    frame.data[0] = qifaId | (1 << 7);
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&wp_qf_resdata);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&wp_qf_resdata)) {
            *Ohm = qifa->Ohm;
            return true;
        }
    }
    return false;
}

void wpQfSetAlarmCode(unsigned char id, unsigned char code) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_SETALARMCODE;
    frame.desid = QIFA_ID(id);
    frame.data[0] = code;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfSetAlarmMask(unsigned char id, unsigned char mask) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_SETALARMMASK;
    frame.desid = QIFA_ID(id);
    frame.data[0] = mask;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}


void wpQfReadAlarm(unsigned char id) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_READALARM;
    frame.desid = QIFA_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}



bool wpQfRead(unsigned char id, unsigned int timeout, unsigned int *status) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_READ;
    frame.desid = QIFA_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&wp_read_qf_flag);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&wp_read_qf_flag)) {
            *status = Sw_Qf_Status;
            return true;
        }
    }
    return false;
}




void canQfRcv(CAN_WP *frame) {
    //if (frame->flag != 1 ) return;
    if (CAN_WP_GET_TYPE(frame->srcid) != CAN_WP_DEV_TYPE_QIFA) {
        return; //device id is not qifa
    }
    unsigned int funcode = frame->funcode;
    unsigned char qfban;
    switch (funcode) {
    case CAN_WP_FUNCODE_QF_READOHM:
        {
            atomicSet(&wp_qf_resdata);
            qfban = CAN_WP_GET_ID(frame->srcid) - 1;
            unsigned char qf = (unsigned char)(frame->data[0]);
            if (qfban >= qifaSys.numofboard || qf >= qifaSys.numperboard) return;
            qifaSys.QiFa_Reg_Table[qfban][qf]->Ohm = (unsigned short)(frame->data[0] >> 8);
        }
        break;
    case CAN_WP_FUNCODE_QF_READALARM:
        break;
    case CAN_WP_FUNCODE_QF_READ:
        atomicSet(&wp_read_qf_flag);
        Sw_Qf_Status = (unsigned short)(frame->data[0]);
        break;
    case CAN_WP_FUNCODE_QF_ALARM:
        qfban = CAN_WP_GET_ID(frame->srcid) - 1;
        if (qfban >= qifaSys.numofboard) return;
        qifawarn[qfban] = (uint16)(frame->data[0] & 0x0f);
        break;
    default:
        break;
    }
}


static QIFA __QiFa_Reg[200]
#if 0
= {
    [SL5_1] = { .name = (wchar_t *)L"[SL5-1]Sel5 Fd1", .board_id = 5, .xuhao = 8, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL6_1] = { .name = (wchar_t *)L"[SL6-1]Sel6 Fd1", .board_id = 4, .xuhao = 1, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL7_1] = { .name = (wchar_t *)L"[SL7-1]Sel7 Fd1", .board_id = 4, .xuhao = 0, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL8_1] = { .name = (wchar_t *)L"[SL8-1]Sel8 Fd1", .board_id = 5, .xuhao = 9, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL9_1] = { .name = (wchar_t *)L"[SL9-1]Sel9 Fd1", .board_id = 6, .xuhao = 7, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL10_1] = { .name = (wchar_t *)L"[SL10-1]Sel10 Fd1", .board_id = 6, .xuhao = 6, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL11_1] = { .name = (wchar_t *)L"[SL11-1]Sel11 Fd1", .board_id = 6, .xuhao = 5, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL12_1] = { .name = (wchar_t *)L"[SL12-1]Sel12 Fd1", .board_id = 6, .xuhao = 4, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL13_1] = { .name = (wchar_t *)L"[SL13-1]Sel13 Fd1", .board_id = 6, .xuhao = 3, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL14_1] = { .name = (wchar_t *)L"[SL14-1]Sel14 Fd1", .board_id = 6, .xuhao = 2, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL15_1] = { .name = (wchar_t *)L"[SL15-1]Sel15 Fd1", .board_id = 6, .xuhao = 1, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL16_1] = { .name = (wchar_t *)L"[SL16-1]Sel16 Fd1", .board_id = 6, .xuhao = 0, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [SL9_2] = { .name = (wchar_t *)L"[SL9-2]Sel9 Fd2", .board_id = 1, .xuhao = 8, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL10_2] = { .name = (wchar_t *)L"[SL10-2]Sel10 Fd2", .board_id = 1, .xuhao = 9, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL11_2] = { .name = (wchar_t *)L"[SL11-2]Sel11 Fd2", .board_id = 1, .xuhao = 0xa, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL12_2] = { .name = (wchar_t *)L"[SL12-2]Sel12 Fd2", .board_id = 1, .xuhao = 0xb, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL13_2] = { .name = (wchar_t *)L"[SL13-2]Sel13 Fd2", .board_id = 1, .xuhao = 0xc, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL14_2] = { .name = (wchar_t *)L"[SL14-2]Sel14 Fd2", .board_id = 1, .xuhao = 0xd, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL15_2] = { .name = (wchar_t *)L"[SL15-2]Sel15 Fd2", .board_id = 1, .xuhao = 0xe, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL16_2] = { .name = (wchar_t *)L"[SL16-2]Sel16 Fd2", .board_id = 1, .xuhao = 0xf, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [SL5_3] = { .name = (wchar_t *)L"[SL5-3]Sel5 Fd3", .board_id = 2, .xuhao = 7, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL6_3] = { .name = (wchar_t *)L"[SL6-3]Sel6 Fd3", .board_id = 3, .xuhao = 0xe, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL7_3] = { .name = (wchar_t *)L"[SL7-3]Sel7 Fd3", .board_id = 3, .xuhao = 0xf, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL8_3] = { .name = (wchar_t *)L"[SL8-3]Sel8 Fd3", .board_id = 5, .xuhao = 0xa, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL9_3] = { .name = (wchar_t *)L"[SL9-3]Sel9 Fd3", .board_id = 1, .xuhao = 0, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL10_3] = { .name = (wchar_t *)L"[SL10-3]Sel10 Fd3", .board_id = 1, .xuhao = 1, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL11_3] = { .name = (wchar_t *)L"[SL11-3]Sel11 Fd3", .board_id = 1, .xuhao = 2, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL12_3] = { .name = (wchar_t *)L"[SL12-3]Sel12 Fd3", .board_id = 1, .xuhao = 3, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL13_3] = { .name = (wchar_t *)L"[SL13-3]Sel13 Fd3", .board_id = 1, .xuhao = 4, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL14_3] = { .name = (wchar_t *)L"[SL14-3]Sel14 Fd3", .board_id = 1, .xuhao = 5, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL15_3] = { .name = (wchar_t *)L"[SL15-3]Sel15 Fd3", .board_id = 1, .xuhao = 6, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL16_3] = { .name = (wchar_t *)L"[SL16-3]Sel16 Fd3", .board_id = 1, .xuhao = 7, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [SL9_4] = { .name = (wchar_t *)L"[SL9-4]Sel9 Fd4", .board_id = 6, .xuhao = 0xf, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL10_4] = { .name = (wchar_t *)L"[SL10-4]Sel10 Fd4", .board_id = 6, .xuhao = 0xe, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL11_4] = { .name = (wchar_t *)L"[SL11-4]Sel11 Fd4", .board_id = 6, .xuhao = 0xd, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL12_4] = { .name = (wchar_t *)L"[SL12-4]Sel12 Fd4", .board_id = 6, .xuhao = 0xc, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL13_4] = { .name = (wchar_t *)L"[SL13-4]Sel13 Fd4", .board_id = 6, .xuhao = 0xb, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL14_4] = { .name = (wchar_t *)L"[SL14-4]Sel14 Fd4", .board_id = 6, .xuhao = 0xa, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL15_4] = { .name = (wchar_t *)L"[SL15-4]Sel15 Fd4", .board_id = 6, .xuhao = 9, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [SL16_4] = { .name = (wchar_t *)L"[SL16-4]Sel16 Fd4", .board_id = 6, .xuhao = 8, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [EV49_Hf2In] = { .name = (wchar_t *)L"[EV49]HfNeedle 2-I", .board_id = 0, .xuhao = 3, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV48_Hf1In] = { .name = (wchar_t *)L"[EV48]HfNeedle 1-I", .board_id = 0, .xuhao = 2, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV47_Hf2Out] = { .name = (wchar_t *)L"[EV47]HfNeedle 2-O", .board_id = 0, .xuhao = 1, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV46_Hf1Out] = { .name = (wchar_t *)L"[EV46]HfNeedle 1-O", .board_id = 0, .xuhao = 0, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [Cam1SC] = { .name = (wchar_t *)L"[S1C]C Cam S  Fd1", .board_id = 4, .xuhao = 0xf, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam1SE] = { .name = (wchar_t *)L"[S1E]E Cam S  Fd1", .board_id = 4, .xuhao = 0xe, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam1TC] = { .name = (wchar_t *)L"[T1C]C Cam T  Fd1", .board_id = 4, .xuhao = 0xd, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam1TE] = { .name = (wchar_t *)L"[T1E]E Cam T  Fd1", .board_id = 4, .xuhao = 0xc, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [Cam2SC] = { .name = (wchar_t *)L"[S2C]C Cam S  Fd2", .board_id = 3, .xuhao = 0x4, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam2SE] = { .name = (wchar_t *)L"[S2E]E Cam S  Fd2", .board_id = 3, .xuhao = 0x5, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam2XE] = { .name = (wchar_t *)L"[X2E]E Cam X  Fd2", .board_id = 3, .xuhao = 0x9, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam2TC] = { .name = (wchar_t *)L"[T2C]C Cam T  Fd2", .board_id = 3, .xuhao = 0x6, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam2TE] = { .name = (wchar_t *)L"[T2E]E Cam T  Fd2", .board_id = 3, .xuhao = 0x7, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [Cam3SC] = { .name = (wchar_t *)L"[S3C]C Cam S  Fd3", .board_id = 3, .xuhao = 0, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam3SE] = { .name = (wchar_t *)L"[S3E]E Cam S  Fd3", .board_id = 3, .xuhao = 1, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam3TC] = { .name = (wchar_t *)L"[T3C]C Cam T  Fd3", .board_id = 3, .xuhao = 2, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam3TE] = { .name = (wchar_t *)L"[T3E]E Cam T  Fd3", .board_id = 3, .xuhao = 3, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [Cam4SC] = { .name = (wchar_t *)L"[S4C]C Cam S  Fd4", .board_id = 4, .xuhao = 0xb, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam4SE] = { .name = (wchar_t *)L"[S4E]E Cam S  Fd4", .board_id = 4, .xuhao = 0xa, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam4XE] = { .name = (wchar_t *)L"[X4E]E Cam X  Fd4", .board_id = 4, .xuhao = 0x6, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam4TC] = { .name = (wchar_t *)L"[T4C]C Cam T  Fd4", .board_id = 4, .xuhao = 0x9, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
    [Cam4TE] = { .name = (wchar_t *)L"[T4E]E Cam T  Fd4", .board_id = 4, .xuhao = 0x8, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 1, },
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [Gu1_1] = { .name = (wchar_t *)L"[GU1-1]Gu1 Fd1", .board_id = 5, .xuhao = 0x7, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu2_1] = { .name = (wchar_t *)L"[GU2-1]Gu2 Fd1", .board_id = 5, .xuhao = 0x6, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu3_1] = { .name = (wchar_t *)L"[GU3-1]Gu3 Fd1", .board_id = 5, .xuhao = 0x5, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu4_1] = { .name = (wchar_t *)L"[GU4-1]Gu4 Fd1", .board_id = 5, .xuhao = 0x4, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu5_1] = { .name = (wchar_t *)L"[GU5-1]Gu5 Fd1", .board_id = 5, .xuhao = 0x3, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu6_1] = { .name = (wchar_t *)L"[GU6-1]Gu6 Fd1", .board_id = 5, .xuhao = 0x2, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu1v1] = { .name = (wchar_t *)L"[GU1v1]Gu1HF Fd1", .board_id = 5, .xuhao = 0x1, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu3v1] = { .name = (wchar_t *)L"[GU3V1]Gu3HF Fd1", .board_id = 5, .xuhao = 0x0, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [Gu1_2] = { .name = (wchar_t *)L"[GU1-2]Gu1 Fd2", .board_id = 2, .xuhao = 0x8, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu2_2] = { .name = (wchar_t *)L"[GU2-2]Gu2 Fd2", .board_id = 2, .xuhao = 0x9, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu3_2] = { .name = (wchar_t *)L"[GU3-2]Gu3 Fd2", .board_id = 2, .xuhao = 0xa, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu4_2] = { .name = (wchar_t *)L"[GU4-2]Gu4 Fd2", .board_id = 2, .xuhao = 0xb, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu5_2] = { .name = (wchar_t *)L"[GU5-2]Gu5 Fd2", .board_id = 2, .xuhao = 0xc, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu6_2] = { .name = (wchar_t *)L"[GU6-2]Gu6 Fd2", .board_id = 2, .xuhao = 0xd, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu1v2] = { .name = (wchar_t *)L"[GU1v2]Gu1HF Fd2", .board_id = 2, .xuhao = 0xe, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu3v2] = { .name = (wchar_t *)L"[GU3V2]Gu3HF Fd2", .board_id = 2, .xuhao = 0xf, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [Gu1_3] = { .name = (wchar_t *)L"[GU1-3]Gu1 Fd3", .board_id = 0, .xuhao = 0x8, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu2_3] = { .name = (wchar_t *)L"[GU2-3]Gu2 Fd3", .board_id = 0, .xuhao = 0x9, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu3_3] = { .name = (wchar_t *)L"[GU3-3]Gu3 Fd3", .board_id = 0, .xuhao = 0xa, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu4_3] = { .name = (wchar_t *)L"[GU4-3]Gu4 Fd3", .board_id = 0, .xuhao = 0xb, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu5_3] = { .name = (wchar_t *)L"[GU5-3]Gu5 Fd3", .board_id = 0, .xuhao = 0xc, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu6_3] = { .name = (wchar_t *)L"[GU6-3]Gu6 Fd3", .board_id = 0, .xuhao = 0xd, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu1v3] = { .name = (wchar_t *)L"[GU1v3]Gu1HF Fd3", .board_id = 0, .xuhao = 0xe, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu3v3] = { .name = (wchar_t *)L"[GU3V3]Gu3HF Fd3", .board_id = 0, .xuhao = 0xf, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [Gu1_4] = { .name = (wchar_t *)L"[GU1-4]Gu1 Fd4", .board_id = 7, .xuhao = 0x7, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu2_4] = { .name = (wchar_t *)L"[GU2-4]Gu2 Fd4", .board_id = 7, .xuhao = 0x6, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu3_4] = { .name = (wchar_t *)L"[GU3-4]Gu3 Fd4", .board_id = 7, .xuhao = 0x5, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu4_4] = { .name = (wchar_t *)L"[GU4-4]Gu4 Fd4", .board_id = 7, .xuhao = 0x4, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu5_4] = { .name = (wchar_t *)L"[GU5-4]Gu5 Fd4", .board_id = 7, .xuhao = 0x3, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu6_4] = { .name = (wchar_t *)L"[GU6-4]Gu6 Fd4", .board_id = 7, .xuhao = 0x2, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu1v4] = { .name = (wchar_t *)L"[GU1v4]Gu1HF Fd4", .board_id = 7, .xuhao = 0x1, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [Gu3v4] = { .name = (wchar_t *)L"[GU3V4]Gu3HF Fd4", .board_id = 7, .xuhao = 0, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [EV5] = { .name = (wchar_t *)L"[EV5]DropCam Fd1", .board_id = 4, .xuhao = 2, .inout = 0, .nc_no = 1,
        .default_nc_no = 1, .nc_no_changeable = 1, .reset_f0_inout = 0, .cam_en = 0, .nc_no_en = 1, },
    [EV17] = { .name = (wchar_t *)L"[EV17]DropCam Fd2", .board_id = 3, .xuhao = 0xd, .inout = 0, .nc_no = 1,
        .default_nc_no = 1, .nc_no_changeable = 1, .reset_f0_inout = 0, .cam_en = 0, .nc_no_en = 1, },
    [EV22] = { .name = (wchar_t *)L"[EV22]DropCam Fd3", .board_id = 3, .xuhao = 0xc, .inout = 0, .nc_no = 1,
        .default_nc_no = 1, .nc_no_changeable = 1, .reset_f0_inout = 0, .cam_en = 0, .nc_no_en = 1, },
    [EV34] = { .name = (wchar_t *)L"[EV34]DropCam Fd4", .board_id = 4, .xuhao = 0x3, .inout = 0, .nc_no = 1,
        .default_nc_no = 1, .nc_no_changeable = 1, .reset_f0_inout = 0, .cam_en = 0, .nc_no_en = 1, },
    [EV35] = { .name = (wchar_t *)L"[EV35]DropHalf-P", .board_id = 7, .xuhao = 0xb, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV36] = { .name = (wchar_t *)L"[EV36]BlowCutters", .board_id = 0, .xuhao = 0x7, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV38] = { .name = (wchar_t *)L"[EV38]Broken-NProbe", .board_id = 5, .xuhao = 0xf, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 1, .cam_en = 0, .nc_no_en = 1, },
    [EV39] = { .name = (wchar_t *)L"[EV39]DepressCam Fd1", .board_id = 4, .xuhao = 0x4, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV40] = { .name = (wchar_t *)L"[EV40]DepressCam Fd2", .board_id = 3, .xuhao = 0xb, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV41] = { .name = (wchar_t *)L"[EV41]DepressCam Fd3", .board_id = 3, .xuhao = 0xa, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV42] = { .name = (wchar_t *)L"[EV42]DepressCam Fd4", .board_id = 4, .xuhao = 0x5, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV43] = { .name = (wchar_t *)L"[EV43]NeedleBlow", .board_id = 2, .xuhao = 0x6, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV50] = { .name = (wchar_t *)L"[EV50]Hp-Clamp Fd1", .board_id = 7, .xuhao = 0xf, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV51] = { .name = (wchar_t *)L"[EV51]Hp-Clamp Fd3", .board_id = 0, .xuhao = 0x4, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV52] = { .name = (wchar_t *)L"[EV52]Lift-Scis Fd1", .board_id = 7, .xuhao = 0xe, .inout = 0, .nc_no = 1,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [EV54] = { .name = (wchar_t *)L"[EV54]Air Blow Pikot", .board_id = 4, .xuhao = 0x7, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV56] = { .name = (wchar_t *)L"[EV56]Blow-Ref-Y", .board_id = 3, .xuhao = 0x8, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV58] = { .name = (wchar_t *)L"[EV58]Machine Needle Opener", .board_id = 2, .xuhao = 0x4, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV59] = { .name = (wchar_t *)L"[EV59]Stop Latches 1", .board_id = 5, .xuhao = 0xd, .inout = 0, .nc_no = 0,
        .default_nc_no = 1, .nc_no_changeable = 0, .reset_f0_inout = 1, .cam_en = 0, .nc_no_en = 1, },
    [EV60] = { .name = (wchar_t *)L"[EV60]Stop Latches 3", .board_id = 2, .xuhao = 0x5, .inout = 0, .nc_no = 0,
        .default_nc_no = 1, .nc_no_changeable = 0, .reset_f0_inout = 1, .cam_en = 0, .nc_no_en = 1, },
    [EV67] = { .name = (wchar_t *)L"[EV67]Clasp Tweezer2", .board_id = 7, .xuhao = 0xd, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, .nc_no_en = 1, },
    [EV68] = { .name = (wchar_t *)L"[EV68]Clasp Tweezer4", .board_id = 7, .xuhao = 0xc, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, .nc_no_en = 1, },

    [EV86] = { .name = (wchar_t *)L"[EV86]Exclusion Knife2", .board_id = 7, .xuhao = 0x9, .inout = 0, .nc_no = 0,
        .default_nc_no = 1, .nc_no_changeable = 0, .reset_f0_inout = 1, .cam_en = 0, .nc_no_en = 1, },
    [EV87] = { .name = (wchar_t *)L"[EV87]Exclusion Knife4", .board_id = 0, .xuhao = 0x6, .inout = 0, .nc_no = 0,
        .default_nc_no = 1, .nc_no_changeable = 0, .reset_f0_inout = 1, .cam_en = 0, .nc_no_en = 1, },
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [EV70] = { .name = (wchar_t *)L"[EV70]Exclusion Knife3", .board_id = 0, .xuhao = 0x5, .inout = 0, .nc_no = 1,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [EV85] = { .name = (wchar_t *)L"[EV85]Clasp Tweezer5", .board_id = 2, .xuhao = 0, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV107] = { .name = (wchar_t *)L"[EV107]Clasp Tweezer6[[EV88]", .board_id = 2, .xuhao = 1, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [EV160] = { .name = (wchar_t *)L"[EV160]movabledropwire Fd1", .board_id = 2, .xuhao = 2, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 1, .cam_en = 0, .nc_no_en = 1, },
    [EV161] = { .name = (wchar_t *)L"[EV161]movabledropwire Fd3", .board_id = 2, .xuhao = 3, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 1, .cam_en = 0, .nc_no_en = 1, },
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    [EV5F] = { .name = (wchar_t *)L"[EV5F]SockBlow", .board_id = 5, .xuhao = 0xb, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EV6F] = { .name = (wchar_t *)L"[EV6F]KnifeBlow", .board_id = 7, .xuhao = 0x8, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
    [EVMAN] = { .name = (wchar_t *)L"[EVMAN]Handle-L", .board_id = 5, .xuhao = 0xC, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, .nc_no_en = 1, },
    [EV45] = { .name = (wchar_t *)L"[EV45]Plasmeca Break", .board_id = 5, .xuhao = 0xe, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 1, .cam_en = 0, },
    [EV84] = { .name = (wchar_t *)L"[EV84]Stop Saw Blade", .board_id = 7, .xuhao = 0xa, .inout = 0, .nc_no = 0,
        .default_nc_no = 0, .nc_no_changeable = 0, .reset_f0_inout = 0, .cam_en = 0, },
}
#endif
;



static RINGBUF qifaringbuf;
static QIFA_VAL qifavalbuf[400];



QIFA_SYS qifaSys;



#if QIFA_USE_MAILBOX==1

static void qifa_mailbox_handler(uint32 queueId, uint32 msg) {
    if (queueId != 0 || msg != 0x55555555) {
        return;
    }
    qifaProcess();
}

#endif


typedef struct {
    unsigned int numofQifa;
    unsigned int numofBoard;
    unsigned int numPerBoard;
    unsigned int qifaOffset;
    unsigned int sizeofQifa;
    unsigned int qifanameOffset;
}QF_HEAD;



typedef struct {
    unsigned short flag;
    unsigned char board_id;
    unsigned char xuhao;
    unsigned int qifa_hao;
    unsigned int default_nc_no : 1;
    unsigned int nc_no_changeable : 1;
    unsigned int nc_no_dis : 1;
    unsigned int iscam : 1;
    unsigned int qifa_name_offset[13];
}QF_PACKED;



#define QIFA_CFG_FILE  L"1:\\sw.qf"

#define QIFA_FLAG    0xaa55

static wchar __qifaname[20000];

static bool __qifainit(QIFA_SYS *qifasys) {
    QIFA *qifa = qifasys->QiFa_Reg = __QiFa_Reg;
    FIL file;
    uint32 rb;
    //int nameoffset;
    QF_HEAD head;
    QF_PACKED qifapack;
    if (f_open(&file, QIFA_CFG_FILE, FA_READ) != FR_OK) {
        return false;
    }
    f_lseek(&file, 0x80);
    if (f_read(&file, &head, sizeof head,&rb) != FR_OK || rb != sizeof head) {
        goto ERROR;
    }
    if (sizeof qifapack > head.sizeofQifa) {
        goto ERROR;
    }
    qifasys->numofboard = head.numofBoard;
    qifasys->numperboard = head.numPerBoard;
    qifasys->numofqifa = head.numofQifa;

    if (f_lseek(&file, head.qifanameOffset) != FR_OK) {
        goto ERROR;
    }
    if (f_read(&file, __qifaname, sizeof __qifaname,&rb) != FR_OK) {
        goto ERROR;
    }
    f_lseek(&file, head.qifaOffset);
    for (int i = 0; i < qifasys->numofqifa; i++) {
        if (f_read(&file, &qifapack, sizeof qifapack,&rb) != FR_OK ||
            sizeof qifapack != rb) {
            goto ERROR;
        }
        if (qifapack.flag != QIFA_FLAG) {
            goto ERROR;
        }
        qifa[i].board_id = qifapack.board_id;
        qifa[i].xuhao = qifapack.xuhao;
        qifa[i].nc_no = qifa[i].default_nc_no = qifapack.default_nc_no;
        qifa[i].nc_no_changeable = qifapack.nc_no_changeable;
        qifa[i].nc_no_display = qifapack.nc_no_dis;
        qifa[i].cam_en = qifapack.iscam;
        qifa[i].flag = QIFA_FLAG;
        for (int j=0;j<13;j++) {
            if (qifapack.qifa_name_offset[j]!=-1UL) {
                qifa[i].name[j] = &__qifaname[qifapack.qifa_name_offset[j] / 2];
            }else{
                qifa[i].name[j] = &__qifaname[qifapack.qifa_name_offset[0] / 2];
            }
        }
    }
    f_close(&file);
    return true;
    ERROR:
    f_close(&file);
    return false;
}



bool qifaInit() {
    if (__qifainit(&qifaSys) == false) {
        return false;
    }
    for (uint32 i = 0; i < qifaSys.numofqifa; i++) {

        ///////////////////////////////////////
        qifaSys.QiFa_Reg[i].flag = QIFA_FLAG;
        //////////////////////////////////////

        if (qifaSys.QiFa_Reg[i].flag != QIFA_FLAG) {
            continue;
        }
        uint32 board = qifaSys.QiFa_Reg[i].board_id;
        uint32 iqifa = qifaSys.QiFa_Reg[i].xuhao;
        if (board >= qifaSys.numofboard || iqifa >= qifaSys.numperboard) {
            return false;
        }
        qifaSys.QiFa_Reg_Table[qifaSys.QiFa_Reg[i].board_id][qifaSys.QiFa_Reg[i].xuhao] = &qifaSys.QiFa_Reg[i];
    }

    ringBufInit(&qifaringbuf, qifavalbuf, sizeof(QIFA_VAL), lenthof(qifavalbuf), 1);

#if QIFA_USE_MAILBOX==1
    mailboxInit(MODULE_ID_MB);
    uint32 mbaddr = modulelist[MODULE_ID_MB].baseAddr;
    MBenableNewMsgInt(mbaddr, 0, 0);
    mbRegistHandler(qifa_mailbox_handler);
#endif
    //////////////////////////////////////
    return true;
}


wchar * qifaName(QIFA *qifa ,uint32 nameindex) {
    if(nameindex>12) nameindex = 0;
    return qifa->name[nameindex];
}


void qifaSet(QIFA *qifa, uint32 val){
    ASSERT(qifa!=NULL && qifa->flag == QIFA_FLAG);
    QIFA_VAL qifa_val;
    qifa_val.qifa = qifa;
    qifa_val.val = !!val;
    while (!ringBufPush(&qifaringbuf, &qifa_val));
#if QIFA_USE_MAILBOX == 1
    uint32 mbaddr = modulelist[MODULE_ID_MB].baseAddr;
    MBsendMessage(mbaddr, 0, 0x55555555);
#endif
}


void qifaSet1(uint32 wpId, uint32 iqifa, uint32 val) {
    ASSERT(wpId > 0 && wpId <= qifaSys.numofboard);
    QIFA *qifa = qifaSys.QiFa_Reg_Table[wpId - 1][iqifa];
    qifaSet(qifa, val);
}


void qifaSetIo(uint32 wpId, uint32 iqifa, uint32 val) {
    ASSERT(wpId > 0 && wpId <= qifaSys.numofboard);
    QIFA *qifa = qifaSys.QiFa_Reg_Table[wpId - 1][iqifa];
    qifaSet(qifa, !!val ^ qifa->nc_no);
}



int32 qifaRead(QIFA * qifa, bool comm) {
    ASSERT(qifa->flag == QIFA_FLAG);
    return qifa->inout;
}

int32 qifaRead1(uint32 wpId, uint32 iqifa, bool comm) {
    ASSERT(wpId > 0 && wpId <= qifaSys.numofboard);
    return qifaSys.QiFa_Reg_Table[wpId - 1][iqifa]->inout;
}

uint32 qifaRead2(uint32 wpId, bool comm) {
    uint32 val = 0;
    ASSERT(wpId > 0 && wpId <= qifaSys.numperboard);
    for (int i = 0; i < qifaSys.numperboard; i++) {
        val |= qifaSys.QiFa_Reg_Table[wpId - 1][i]->inout << i;
    }
    return val;
}


int32 qifaReadIo(QIFA * qifa, bool comm) {
    ASSERT(qifa!=NULL && qifa->flag == QIFA_FLAG);
    return(!!qifa->inout) ^ (!!qifa->nc_no);
}

int32 qifaReadIo1(uint32 wpId, uint32 iqifa, bool comm) {
    ASSERT(wpId > 0 && wpId <= qifaSys.numofboard);
    QIFA *qifa = qifaSys.QiFa_Reg_Table[wpId - 1][iqifa];
    return qifaReadIo(qifa, comm);
}

uint32 qifaReadIo2(uint32 wpId, bool comm) {
    uint32 val = 0;
    ASSERT(wpId > 0 && wpId <= qifaSys.numperboard);
    for (int i = 0; i < qifaSys.numperboard; i++) {
        val |= qifaReadIo1(wpId, i, comm) << i;
    }
    return val;
}



/*uint16 qifaReadWarn(uint32 wpId){
    uint16 tmp = qifawarn[wpId-1];
    if(tmp!=0){
        qifawarn[wpId-1] = 0;
    }
    return tmp;
}*/


uint32 qifaBak(QIFA * qifa) {
    ASSERT(qifa!=NULL && qifa->flag == QIFA_FLAG);
    return qifa->inout_bak = qifa->inout;
}

uint32 qifaBak2(uint32 wpId) {
    uint32 val = 0;
    for (int i = 0; i < qifaSys.numperboard; i++) {
        val |= qifaBak(qifaSys.QiFa_Reg_Table[wpId - 1][i]) << i;
    }
    return val;
}


void qifaRestore(QIFA * qifa) {
    ASSERT(qifa!=NULL && qifa->flag == QIFA_FLAG);
    qifaSet(qifa, qifa->inout_bak);
}


void qifaRestore2(uint32 wpId) {
    for (int i = 0; i < qifaSys.numperboard; i++) {
        qifaRestore(qifaSys.QiFa_Reg_Table[wpId - 1][i]);
    }
}

#define QIFA_BOARD_NUM        10


#define QIFA_GUILEI_BUF_LEN  200
typedef struct {
    unsigned char data_value[QIFA_GUILEI_BUF_LEN];
    int num;
}Qifa_Guilei;


void qifaProcess() {
    static Qifa_Guilei qifaguilei[QIFA_BOARD_NUM];
    QIFA_VAL qifaval;
    QIFA *qifa;
    int val;
    while (1) {
        if (!ringBufPop(&qifaringbuf, &qifaval)) {
            break;
        }
        qifa = qifaval.qifa;
        ASSERT(qifa!=NULL && qifa->flag == QIFA_FLAG);
        val = qifa->inout = qifaval.val;
        qifaguilei[qifa->board_id].data_value[qifaguilei[qifa->board_id].num]
        = (uint8)((val ^ !!qifa->nc_no) << 7 | qifa->xuhao);
        qifaguilei[qifa->board_id].num++;
        if (qifaguilei[qifa->board_id].num >= QIFA_GUILEI_BUF_LEN) {
            //todo warning
            break;
        }
    }
    for (int i = 0; i < qifaSys.numofboard; i++) {
        if (qifaguilei[i].num != 0) {
            int32 j = 0;
            do {
                uint32 datanum = MIN(qifaguilei[i].num, 8);
                wpQfWrite(i + 1, &qifaguilei[i].data_value[j], datanum);
                j += datanum;
                qifaguilei[i].num -= datanum;
            }while (qifaguilei[i].num);
        }
    }
}
