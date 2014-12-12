#ifndef __CO__H__
#define __CO__H__


#include "type.h"
#include "list.h"
#include "ff.h"
#include "co.h"


__packed typedef struct {
    uint16 sizet;
    uint32 diameter;
    uint16 niddle;
    uint8 dummy[8];
    uint32 co_size;
    uint8 unknow[8];
    uint32 sizemotorAddr;
    uint32 sinkermotor_feed1_3_Addr;
    uint32 stitch1Addr;
    uint32 stitch2Addr;
    uint32 stitch3Addr;
    uint32 stitch4Addr;
    uint32 sinkermotor_feed2_4_Addr;
    uint32 sinkerangular_Addr;
    uint32 unknow1;
}CO_PART1_ATTRIB;



__packed typedef struct {
    char unknow[32];
    uint32 pyf1addr;
    uint32 pyf2addr;
    uint32 pyf3addr;
    uint32 pyf4addr;
    uint32 dty1addr;
    uint32 dty2addr;
    uint32 dty3addr;
    uint32 dty4addr;
    uint32 yoyo1addr;
    uint32 yoyo2addr;
    uint32 yoyo3addr;
    uint32 yoyo4addr;
    uint32 yoyo5addr;
    uint32 yoyo6addr;
    uint32 yoyo7addr;
    uint32 yoyo8addr;
    uint32 yoyo9addr;
    uint32 yoyo10addr;
    uint32 yoyo11addr;
    uint32 yoyo12addr;
    uint32 unkownaddr[12];
}CO_PART2_ATTRIB;


__packed typedef struct {
    char unknow[40];
    uint32 func_addr;
    char unknow1[8];
    uint32 speed_addr;
    char unknow2[8];
}CO_PART3_ATTRIB;


__packed typedef struct {
    char unkown[56];
    uint32 fengmen1addr;
    uint32 fengmen2addr;
}CO_PART4_ATTRIB;


__packed typedef struct {
    uint16 beginFlag; //0xffff
    uint16 angle;
    uint16 beginStep;
    uint16 endStep;
    uint16 groupNum;
    uint16 unkown;  //0x0000
}CO_MOTORHEAD;


typedef struct {
    uint16 beginStep;
    uint16 endStep;
    uint16 angle;
    uint16 groupNum;
}MOTORHEAD;


__packed typedef struct {
    uint16 start[3];     //motor duplicate 3 times
    uint16 startWidth;
    uint16 startWidthDec; //dec: 0-0,1-0.25 3-0.75
    uint16 end[3];  //cylinder duplicate 3 times
    uint16 endWidth;
    uint16 endWidthDec; //dec: 0-0,1-0.25 3-0.75
}CO_SIZEMOTORPARAM;


typedef struct {
    uint16 start;     //motor duplicate 3 times
    uint16 startWidth;
    uint16 startWidthDec; //dec: 0-0,1-0.25 3-0.75
    uint16 end;  //cylinder duplicate 3 times
    uint16 endWidth;
    uint16 endWidthDec; //dec: 0-0,1-0.25 3-0.75
}SIZEMOTORPARAM;

__packed typedef struct {
    unsigned int size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    CO_MOTORHEAD head;
    CO_SIZEMOTORPARAM param[8];
}CO_SIZEMOTOR_ZONE;


typedef struct {
    char descrpition[24];
    MOTORHEAD head;
    SIZEMOTORPARAM param[8];
    struct list_head list;
}SIZEMOTOR_ZONE;



__packed typedef struct {
    uint16 qi_feed[3];
    char unknow[4];
    uint16 qf_feed[3];
    char unknow1[4];
}CO_SINKERMOTORPARAM;

__packed typedef struct {
    uint32 size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    CO_MOTORHEAD head;
    CO_SINKERMOTORPARAM param[8];
}CO_SINKERMOTOR_ZONE;


typedef struct {
    uint16 qi_feed;
    uint16 qf_feed;
}SINKERMOTORPARAM;

typedef struct {
    char descrpition[24];
    MOTORHEAD head;
    SINKERMOTORPARAM param[8];
}SINKERMOTOR_ZONE;

typedef struct {
    uint16 tr1[3];
    char unknow[4];
    uint16 _tr1[3];
    char unknow1[4];
}STITCHCAMSPARAM;

typedef struct {
    uint32 size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    MOTORHEAD head;
    STITCHCAMSPARAM param[8];
}STITCHCAMS_ZONE;




__packed typedef struct {
    uint32 step;
    uint32 rpm[2];
    uint16 ramp[8][2];
}CO_SPEED;



typedef struct {
    uint32 step;
    uint32 rpm;
    uint16 ramp[8];
    struct list_head list;
}SPEED;



typedef struct {
    uint16 angular;
    uint16 unknow;
    uint16 add[4];
    struct list_head list;
}FUNC;




__packed typedef struct {
    uint16 ryoyo;
    uint16 zyoyo;
    uint16 yyoyo;
    uint16 dummy[3];
    uint16 series;
}MOTOR_HEADER_PARAM;




typedef struct {
    uint16 angular;
    uint16 dummy;
    uint16 funcode;
    uint16 val;
    struct list_head list;
}FENGMEN;



__packed typedef struct{
    uint16 begin;
    uint16 end;
    uint16 economize[8][2];
}CO_ECONOMIZER_PARAM;


 typedef struct{
    uint16 begin;
    uint16 end;
    uint16 economize[8];
}ECONOMIZER_PARAM;


__packed typedef struct{
    uint32 flag;
    uint32 headsize;
    uint32 unkown[5];
    uint32 size;
    uint32 unkown1[15];
}CO_ECONOMIZER_HEAD;



struct __CO_RUN ;

typedef struct {
    uint32 diameter;
    uint16 niddle;
    uint32 numofspeed;
    uint32 numofstep;
    uint32 file_speedOff;
    SPEED speed[100];
    uint32 numofsizemotorzone;
    SIZEMOTOR_ZONE  sizemotor[20];
    uint32 numofsinkmoterzone_1_3;
    SINKERMOTOR_ZONE  sinkmoterzone_1_3[20];
    uint32 numofsinkmoterzone_2_4;
    SINKERMOTOR_ZONE  sinkmoterzone_2_4[20];
    uint32 numofsinkangular;
    SINKERMOTOR_ZONE  sinkangular[20];
    MOTOR_HEADER_PARAM motor_header[12];
    struct list_head func[500];
    struct list_head fengmen[500];
    uint32 file_econoOff;
    uint32 numofeconomizer;
    ECONOMIZER_PARAM econo[200];
    struct __CO_RUN *run;
    //sink sinkermotor
}CO;



typedef struct{
    uint16 econoFlag;
    ECONOMIZER_PARAM  *econo;
    SPEED *speed;
    SIZEMOTOR_ZONE *sizemotor;
    SINKERMOTOR_ZONE  *sinkmoterzone_1_3;
    struct list_head *func;
    struct list_head *fengmen;
    struct list_head list;
}CO_RUN_STEP;

#define ECONO_BEGIN   2
#define ECONO_END     4
#define IS_ECONO_BEGIN(A)   ((A).econoFlag & ECONO_BEGIN)   
#define IS_ECONO_END(A)   ((A).econoFlag & ECONO_END) 




typedef struct __CO_RUN{
    uint16 istep;                   //step conter when run;
    uint16 nextline;
    uint16 nextstep;                //nextstep != istep+1, due to economizer
    uint16 prerpm;
    uint16 rpm;
    uint16 iecono;                  //current economizer counter
    uint16 econonum;                //economizer
    uint16 econostepfrom;           //economizer begin step
    uint16 econostepto;             //economizer end step(end>begin)
    uint16 numofline[8];            //number of line ,>=numofstep due to economizer
    uint16 numofstep;               //number offset step ,equal to CO::numofstep
    int16  speedAcc;                //speed acceleration when run
    uint16 targetSpeed;             //target speed when ramp
    CO *co;                         //poit to associated co
    CO_RUN_STEP *stepptr[200];      //point to CO_RUN_STEP list element
    struct list_head step;          //CO_RUN_STEP list
}CO_RUN;




typedef struct {
    uint16 istep;
    uint16 iecono;
    uint16 econonum;
    uint16 iline;
    uint16 rpm;
    uint32 sizemotor;
    bool willAct;
}CO_RUN_LINE;





extern void coInit();
extern bool coParas(const TCHAR *path, CO *co, unsigned int *offset);
extern void coRelease(CO *co);
extern void coCreateIndex(CO_RUN *co_run, CO *co);
extern void coRun(CO_RUN *co_run);
extern uint32 corunReadLine(CO_RUN *co_run, CO_RUN_LINE *line, uint32 size);
extern void coRelease(CO *co);

#endif
