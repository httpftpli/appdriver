
#include "can_wp.h"
#include "pf_can.h"
#include "qifa.h"
#include "module.h"
#include "debug.h"
#include <string.h>
#include "atomic.h"
#include "pf_timertick.h"

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


#define QIFA_ID(ID)    (CAN_WP_DEV_TYPE_QIFA|ID)
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static atomic wp_read_qf_flag,wp_qf_resdata;
static unsigned short Sw_Qf_Status;

S_Valve_Status SW_Value_Status[Max_Output_Test_Page];
S_QiFa_ResData  QiFa_ResData;
//static unsigned short Sw_Qf_Resdata;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


void wpQfPowrSave(unsigned char  id, bool on)
{
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_POWERSAVE;
      frame.desid = QIFA_ID(id);
      frame.data[0] = (unsigned char)on;
      frame.dlc = 1;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfEn(unsigned char  id, bool on)
{
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_EN;
      frame.desid = QIFA_ID(id);
      frame.data[0] = (unsigned char)on;
      frame.dlc = 1;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfRst(unsigned char  id, bool rst)
{
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_RESET;
      frame.desid = QIFA_ID(id);
      frame.data[0] = (unsigned char)(!!rst);
      frame.dlc = 1;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfSetDuty(unsigned char  id, unsigned char duty)
{
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_SETDUTY;
      frame.desid = QIFA_ID(id);
      frame.data[0] = (unsigned char)duty;
      frame.dlc = 1;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}



void wpQfWrite(unsigned char id, unsigned char *val, unsigned char count)
{
      ASSERT(count < 9);
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_WRITE;
      frame.desid = QIFA_ID(id);
      memcpy(frame.data, val, count);
      //frame.data[0]=val;
      frame.dlc = count;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQFRecover(unsigned char id, unsigned short recover_data)
{
      unsigned  char val[8];
      for (unsigned char m = 0; m < 2; m++)
      {
	    for (unsigned char n = 0; n < 8; n++)
	    {
		  unsigned short zhuangtai = 0;
		  if (m == 0)
		  {
			zhuangtai = (recover_data & 0xff);
		  }
		  else
		  {
			zhuangtai = ((recover_data & 0xff00) >> 8);
		  }
		  if (zhuangtai & (0x01 << n))
		  {
			val[n] = (0x80 | (n + m * 8));
		  }
		  else
		  {
			val[n] = (0x7f & (n + m * 8));
		  }
	    }
	    wpQfWrite(id, val, 8);
      }
}



bool wpQfReadOhm(unsigned char id, unsigned char qifaId, unsigned short timeout)
{
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_READOHM;
      frame.desid = QIFA_ID(id);
      frame.data[0] = qifaId | (1 << 8);
      frame.dlc = 1;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
      atomicClear(&wp_qf_resdata);
      withintimedo(tmark, timeout)
      {
	    if (atomicTestClear(&wp_qf_resdata))
	    {
		  return true;
	    }
      }
      return false;
}

void wpQfSetAlarmCode(unsigned char id, unsigned char code)
{
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_SETALARMCODE;
      frame.desid = QIFA_ID(id);
      frame.data[0] = code;
      frame.dlc = 1;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfSetAlarmMask(unsigned char id, unsigned char mask)
{
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_SETALARMMASK;
      frame.desid = QIFA_ID(id);
      frame.data[0] = mask;
      frame.dlc = 1;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}


void wpQfReadAlarm(unsigned char id)
{
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_READALARM;
      frame.desid = QIFA_ID(id);
      frame.dlc = 0;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

bool wpQfRead(unsigned char id, unsigned int timeout, unsigned int *status)
{
      DEFINE_CAN_WP_FRAME(frame);
      frame.funcode = CAN_WP_FUNCODE_QF_READ;
      frame.desid = QIFA_ID(id);
      frame.dlc = 0;
      CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
      atomicClear(&wp_read_qf_flag);
      withintimedo(tmark, timeout)
      {
	    if (atomicTestClear(&wp_read_qf_flag))
	    {
		  *status 	= Sw_Qf_Status;
		  return true;
	    }
      }
      return false;
}

//static unsigned short qfOhm[10][16];
void canQfRcv(CAN_WP *frame)
{
      //if (frame->flag != 1 ) return;
      if ((frame->srcid & 0xf0) != CAN_WP_DEV_TYPE_QIFA)
      {
	    return; //device id is not qifa
      }
      unsigned int funcode = frame->funcode;
      unsigned char qfban;
      switch (funcode)
      {
	  case CAN_WP_FUNCODE_QF_READOHM:
	  {
		atomicSet(&wp_qf_resdata);
		qfban = (frame->srcid & 0xf) - 1;
		unsigned char qf = (unsigned char)(frame->data[0]);
		if ((qfban > 10) || (qf > 16)) return;
		//qfOhm[qfban][qf] = (unsigned short) (frame->data[0]>>8);
		SW_Value_Status[qfban].Now_Value_Res[qf] = (unsigned short)(frame->data[0] >> 8);
		QiFa_ResData.board_xuhao = qfban;
		QiFa_ResData.qifa_num = qf;
		QiFa_ResData.res_fresh = true;
		//QiFa_ResData.qifa_resdata =  SW_Value_Status[qfban].Now_Value_Res[qf] ;
	  }
		break;
	  case CAN_WP_FUNCODE_QF_READALARM:
		break;
	  case CAN_WP_FUNCODE_QF_READ:
		atomicSet(&wp_read_qf_flag);
		Sw_Qf_Status = (unsigned short)(frame->data[0]);
		break;
	  case CAN_WP_FUNCODE_QF_ALARM:
		qfban = (frame->srcid & 0xf) - 1;
		if (qfban > 10) return;
		SW_Value_Status[qfban].Value_ERR = (~((unsigned char)(frame->data[0])))&0xf;
		break;
	  default:
		break;
      }
}
