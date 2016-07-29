
#ifndef __QIFA__H__
#define __QIFA__H__

#include <stdbool.h>
#include <wchar.h>
#include "type.h"
#include "algorithm.h"
#include "can_dmp.h"
#include "ff.h"





typedef struct
{
    //uint32 id;
    wchar_t *name[13];                //NAME
    char nickname[10];
    uint16 qfid;
    unsigned char board_id;     //0~10
    unsigned char xuhao;        //0~15
    unsigned short inout:1;
    unsigned short inout_bak:1;
    unsigned short nc_no:1;
    unsigned short default_nc_no:1;
    unsigned short nc_no_changeable:1;
    unsigned short nc_no_display:1;
    unsigned short reset_f0_inout:1;
    unsigned short cam_en:1;
    unsigned short flag;
    unsigned short Ohm;
}QIFA;


typedef struct{
    unsigned int numOfQifa;
    QIFA qifa[16];
    DMP_DEV *dmpdev;
}QIFA_BOARD;


typedef struct {
    QIFA *QiFa_Reg[0x400];
    QIFA *QiFa_Reg_Table[30][16];
    QIFA_BOARD *board[30];
    uint32 numofqifa;
    uint32 numofboard;
    uint32 numperboard;
    uint32 flag;
}QIFA_SYS;


extern  QIFA_SYS qifaSys;
extern  uint16 qifawarn[30];



typedef struct {
    QIFA *qifa;
    uint32 val;
}QIFA_VAL;





extern void wpQfPowrSave(unsigned char  id, bool on);
extern void wpQfEn(unsigned char  id, bool on);
extern void wpQfRst(unsigned char  id, bool rst);
extern void wpQfSetDuty(unsigned char  id, unsigned char duty);
extern void wpQfSetAlarmCode(unsigned char id, unsigned char code);
extern void wpQfSetAlarmMask(unsigned char id, unsigned char mask);
extern void wpQfReadAlarm(unsigned char id);
extern bool wpQfRead(unsigned char id, unsigned int timeout, unsigned int *status);
extern bool wpQfReadOhm(unsigned char id, unsigned char qifaId, unsigned short timeout,unsigned short *Ohm);
extern void wpQfWrite(unsigned char id, unsigned char *val, unsigned char count);
extern void wpRunIsr_QfWrite(RINGBUF * ringBuf,unsigned char id, unsigned char *val, unsigned char count);

extern bool qifaInit(TCHAR *path);
extern void qifaSet(QIFA *qifa,uint32 val);
extern void qifaFunSet(uint16 funcode,uint32 val);
extern const char *qifaNickName(uint16 valecode);
extern const wchar * qifaName(QIFA *qifa ,uint32 nameindex);
extern void qifaSet1(uint32 boardId,uint32 iqifa, uint32 val);
extern void qifaSetIo(uint32 wpId, uint32 iqifa, uint32 val);
extern int32 qifaRead(QIFA *qifa,bool comm);
extern int32 qifaRead1(uint32 boardId,uint32 iqifa,bool comm);
extern uint32 qifaRead2(uint32 boardId, bool comm);
extern int32 qifaReadIo(QIFA *qifa, bool comm);
extern int32 qifaReadIo1(uint32 wpId,uint32 iqifa,bool comm);
extern uint32 qifaReadIo2(uint32 wpId, bool comm);
extern uint16 qifaReadWarn(uint32 wpId);



extern uint32 qifaBak(QIFA *qifa);
extern void qifaRestore(QIFA *qifa);


extern uint32 qifaBak2(uint32 wpId);
extern void qifaRestore2(uint32 wpId);

extern void qifaProcess();

#endif /* __QIFA__H__*/




