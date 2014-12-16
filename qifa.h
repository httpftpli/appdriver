
#ifndef __QIFA__H__
#define __QIFA__H__

#include <stdbool.h>

#define Max_Output_Test_Page		10
#define Max_Page_Value_Num		16		//ÿҳ������������Ŀ
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//������ֵ�ṹ
typedef struct
{
	  bool res_fresh;
	  unsigned char board_xuhao;
	  unsigned char qifa_num;
	  //unsigned short qifa_resdata;
}S_QiFa_ResData;
//˿���������״̬
typedef struct
{
	  unsigned short  Now_Value_Status;               //��ǰ����״ֵ̬  ��λ�洢 Ϊ1 ��ʾON  Ϊ0 ��ʾOFF
	  unsigned short  Ago_Value_Status;               //ԭ������״ֵ̬  ��λ�洢 Ϊ1 ��ʾON  Ϊ0 ��ʾOFF
	  unsigned short  Now_Value_Res[16];              //��ǰ������ֵ  ���Ե���������ֵ����ֵ
	  unsigned short  Save_Value_Res[16];             //�洢��������ֵ
	  unsigned char  Value_ERR;                       //��������  4������Ϊһ������

}S_Valve_Status;


extern S_QiFa_ResData  QiFa_ResData;
extern S_Valve_Status SW_Value_Status[Max_Output_Test_Page];


//extern void canQfRcv(CAN_WP *frame);
extern void wpQfPowrSave(unsigned char  id, bool on);
extern void wpQfEn(unsigned char  id, bool on);
extern void wpQfRst(unsigned char  id, bool rst);
extern void wpQfSetDuty(unsigned char  id, unsigned char duty);
//extern void wpQfWrite(unsigned char id, unsigned char *val,unsigned int count );
extern void wpQfSetAlarmCode(unsigned char id, unsigned char code);
extern void wpQfSetAlarmMask(unsigned char id, unsigned char mask);
extern void wpQfReadAlarm(unsigned char id);
extern bool wpQfRead(unsigned char id, unsigned int timeout, unsigned int *status);
extern bool wpQfReadOhm(unsigned char id, unsigned char qifaId, unsigned short timeout);
extern void wpQfWrite(unsigned char id, unsigned char *val, unsigned char count);
extern void wpQFRecover(unsigned char id, unsigned short recover_data);

#endif /* __QIFA__H__*/

