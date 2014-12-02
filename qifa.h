
#ifndef __QIFA__H__
#define __QIFA__H__

#include <stdbool.h>

extern void wpQfPowrSave(unsigned char  id, bool on);
extern void wpQfEn(unsigned char  id, bool on);
extern void wpQfRst(unsigned char  id,bool rst);
extern void wpQfSetDuty(unsigned char  id, unsigned char duty);
extern void wpQfWrite(unsigned char id, unsigned char *val,unsigned int count );
extern void wpQfReadOhm(unsigned char id, unsigned char qifaId );
extern void wpQfSetAlarmCode(unsigned char id,unsigned char code);
extern void wpQfSetAlarmMask(unsigned char id,unsigned char mask);
extern void wpQfReadAlarm(unsigned char id);
extern void wpQfRead(unsigned char id);

#endif /* __QIFA__H__*/

