#ifndef __CO__H__
#define __CO__H__


#include "type.h"
#include "list.h"
#include "ff.h"
#include "co.h"


#define FEED_NUMBER   6

#define SEL_BASE   0
#define SEL_LINE_NUMBER   32
#define HAFUZHEN_NUMBER   32
#define HAFUZHEN_BASE     SEL_BASE + FEED_NUMBER * SEL_LINE_NUMBER
#define CAM_BASE          HAFUZHEN_BASE + HAFUZHEN_NUMBER
#define CAM_LINE_NUMBER   16
#define YARN_FINGER_BASE  CAM_BASE + FEED_NUMBER*CAM_LINE_NUMBER
#define YARN_LINE_NUMBER  16
#define VALVE_MIS_BASE    YARN_FINGER_BASE + FEED_NUMBER*YARN_LINE_NUMBER
#define VALVE_MIS_NUMBER  64



typedef __packed struct {
    uint16 unkown;
    char name[4];
    uint16 offset;
    uint16 len;
    char unkown2[17];
    uint32 check;
}CO_SECTION;


typedef __packed struct {
    uint32 coCheck;
    char coFileName[11];
    char machineName[11];
    int32 softver;
    char unkown[44];
    CO_SECTION sec[14];
    char unkown1[4];
}CO_HEADER ;


__packed typedef struct
{
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
}
CO_RESET_INFO;



__packed typedef struct
{
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
}
CO_PART2_ATTRIB;


__packed typedef struct
{
    char unknow[40];
    uint32 func_addr;
    char unknow1[8];
    uint32 speed_addr;
    char unknow2[8];
}
CO_CATE_INFO;


__packed typedef struct
{
    char unkown[56];
    uint32 fengmen1addr;
    uint32 fengmen2addr;
}
CO_MPP_INFO;


__packed typedef struct
{
    uint16 beginFlag; //0xffff
    uint16 angle;
    uint16 beginStep;
    uint16 endStep;
    uint16 groupNum;
    uint16 unkown;  //0x0000
}
CO_MOTORHEAD;


typedef struct
{
    uint16 beginStep;
    uint16 endStep;
    uint16 angle;
    uint16 groupNum;
}MOTORHEAD;


__packed typedef struct
{
    uint16 start[3];     //motor duplicate 3 times
    uint16 startWidth;
    uint16 startWidthDec; //dec: 0-0,1-0.25 3-0.75
    uint16 end[3];        //cylinder duplicate 3 times
    uint16 endWidth;
    uint16 endWidthDec; //dec: 0-0,1-0.25 3-0.75
}
CO_SIZEMOTORPARAM;


typedef struct
{
    int32 start;
    uint16 startWidth;
    uint16 startWidthDec; //dec: 0-0,1-0.25 3-0.75
    int32 end;
    uint16 endWidth;
    uint16 endWidthDec; //dec: 0-0,1-0.25 3-0.75
    int32 acc;
}SIZEMOTORPARAM;

__packed typedef struct
{
    unsigned int size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    CO_MOTORHEAD head;
    CO_SIZEMOTORPARAM param[8];
}
CO_SIZEMOTOR_ZONE;


typedef struct
{
    char descrpition[24];
    MOTORHEAD head;
    SIZEMOTORPARAM param[8];
    struct list_head list;
}SIZEMOTOR_ZONE;



__packed typedef struct
{
    uint16 qi_feed[3];
    char unknow[4];
    uint16 qf_feed[3];
    char unknow1[4];
}
CO_SINKERMOTORPARAM;

__packed typedef struct
{
    uint32 size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    CO_MOTORHEAD head;
    CO_SINKERMOTORPARAM param[8];
}
CO_SINKERMOTOR_ZONE;


typedef struct
{
    uint32 qi_feed;
    uint32 qf_feed;
    int32 acc;
}SINKERMOTORPARAM;

typedef struct
{
    char descrpition[24];
    MOTORHEAD head;
    SINKERMOTORPARAM param[8];
}SINKERMOTOR_ZONE;

typedef struct
{
    uint16 tr1[3];
    char unknow[4];
    uint16 _tr1[3];
    char unknow1[4];
}STITCHCAMSPARAM;

typedef struct
{
    uint32 size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    MOTORHEAD head;
    STITCHCAMSPARAM param[8];
}STITCHCAMS_ZONE;




__packed typedef struct
{
    uint32 step;
    uint32 rpm[2];
    uint16 ramp[8][2];
}
CO_SPEED;



typedef struct
{
    int32 step;
    //int32 prerpm[8];
    //int32 acc[8];
    int32 rpm;
    int16 ramp[8];
    bool coed;
    struct list_head list;
}SPEED;


__packed typedef struct
{
    uint16 angular;
    uint16 value;
    uint16 funcode;
    uint8 add[4];
}CO_FUNC;



typedef struct
{
    uint16 angular;
    uint16 value;
    uint16 funcode;
    uint8 add[4];
    struct list_head list;
}FUNC;




__packed typedef struct
{
    uint16 ryoyo;
    uint16 zyoyo;
    uint16 yyoyo;
    uint16 dummy[3];
    uint16 series;
}
MOTOR_HEADER_PARAM;


typedef struct
{
    uint16 angular;
    uint16 dummy;
    uint16 funcode;
    uint16 val;
}CO_FENGMEN;


typedef struct
{
    uint16 angular;
    uint16 dummy;
    uint16 funcode;
    uint16 val;
    struct list_head list;
}FENGMEN;



__packed typedef struct
{
    uint16 begin;
    uint16 end;
    uint16 economize[8][2];
}
CO_ECONOMIZER_PARAM;


typedef struct
{
    uint16 begin;
    uint16 end;
    uint16 economize[8];
}ECONOMIZER_PARAM;




__packed typedef struct{
    uint32 flag;
    uint32 unkown;
    char   headmppctrl;
    char   dummy[3];
    uint32 unkown1[4];
    uint32 filedescOffset;
    uint32 weltAddrOffset;
    uint32 econoAddrOffset;
    uint32 unkown3;
    uint32 supesize;
    char unkown4[16];
}CO_SUPE_INFO;



__packed typedef struct{
    uint16 weltflag;
    uint16 unknow;
    uint16 step;
}CO_WELT_PARAM ;

typedef struct {
    uint16 weltinstep;
    uint16 weltoutstep;
    struct list_head list;
}WELT_PARAM;


typedef struct __S_CO_RUN S_CO_RUN;

typedef struct
{
    CO_HEADER head;
    uint32 diameter;
    uint16 niddle;
    uint32 numofstep;

    CO_RESET_INFO resetinfo;
    CO_CATE_INFO  cateinfo;
    CO_SUPE_INFO  supeinfo;

    uint32 numofsizemotorzone;
    SIZEMOTOR_ZONE sizemotor[30];
    uint32 numofsinkmoterzone_1_3;
    SINKERMOTOR_ZONE sinkmoterzone_1_3[30];
    uint32 numofsinkmoterzone_2_4;
    SINKERMOTOR_ZONE sinkmoterzone_2_4[30];
    uint32 numofsinkangular;
    SINKERMOTOR_ZONE sinkangular[30];
    MOTOR_HEADER_PARAM motor_header[12];
    struct list_head speed;
    struct list_head func[500];
    struct list_head fengmen[500];
    struct list_head welt;
    uint32 numofeconomizer;
    ECONOMIZER_PARAM econo[200];
    S_CO_RUN *run;
    //sink sinkermotor
}S_CO;



typedef struct
{
    uint32 ilinetag[8];
    //uint32 ilineEtag[8];
    uint16 econoFlag;
    ECONOMIZER_PARAM *econo;

    bool welt;

    SPEED *speed;

    SIZEMOTOR_ZONE *sizemotor;
    SINKERMOTOR_ZONE *sinkmoterzone_1_3;
    SINKERMOTOR_ZONE *sinkmoterzone_2_4;
    struct list_head *func;
    struct list_head *fengmen;
    struct list_head list;
}S_CO_RUN_STEP;



#define ECONO_BEGIN   2
#define ECONO_END     4
#define IS_ECONO_BEGIN(A)   ((A).econoFlag & ECONO_BEGIN)
#define IS_ECONO_END(A)   ((A).econoFlag & ECONO_END)
#define IS_ECONO_BEGIN_END(A)  (((A).econoFlag & ECONO_END)&&((A).econoFlag & ECONO_BEGIN))



typedef struct {
    uint16 num;
    uint16 valvecode[64];
}ACT_GROUP;


typedef struct{
    uint16 num;
    uint16 alarmcode[10];
}ALARM_GROUP;


typedef struct __S_CO_RUN
{
    int32 istep;                   //step conter when run;
    uint32 nextline;                //下一行
    int32 nextstep;                //nextstep != istep+1, due to economizer
    int32 prestep;

    uint16 prerpm;
    int32 rpm;
    int16 speedAcc;                 //speed acceleration when run
    uint16 targetSpeed;             //target speed when ramp

    bool welt;                      //is_or_not in welt flag

    uint32 sizemotor;
    uint32 sinkmotor1_3;
    uint32 sinkmotor2_4;
   // FENGMEN *fengmen[360];

    uint16 iecono;                  //current economizer counter
    uint16 econonum;                //economizer
    uint16 econostepfrom;           //economizer begin step
    uint16 econostepto;             //economizer end step(end>begin)
                                    //
    uint16 numofline[8];            //number of line ,>=numofstep due to economizer   总圈数
    uint16 numofstep;               //number offset step ,equal to CO::numofstep		总步数
    S_CO *co;                         //poit to associated co
    S_CO_RUN_STEP *stepptr[200];      //point to S_CO_RUN_STEP list element
    struct list_head step;          //CO_RUN_STEP list
}S_CO_RUN;


#define LINE_FLAG_F4  0x01
#define LINE_FLAG_ACT 0x02


typedef struct
{
    uint16 istep;                    //当前STEP
    uint32 iline;                    //实际当前圈数

    bool welt;                      //flag is_or_not in welt;

    uint16 iecono;                   //当前循环
    uint16 econobegin;              //当前循环首,如果没有循环为0
    uint16 econoend;                //当前循环尾,如果没有循环为0
    uint16 econonum;                 //当前循环总共循环

    bool isfengmenAct;
    ACT_GROUP act[360];
    ALARM_GROUP alarm[360];
    FENGMEN *fengmen[360];

    uint16 rpm;                        //当前圈设定速度
    char *zonename;
    uint16 zonebegin;
    uint16 zoneend;
    uint32 sizemotor;                 //步进电机值
    uint32 sinkmotor1_3;               //sinker motor
    uint32 sinkmotor2_4;
    uint32 flag;
}S_CO_RUN_LINE;


typedef struct {
    char filename[12];
    uint32 num;
}S_CN_GROUP;


__packed typedef  struct {
    char co[12];
    short unknow;
    uint32 product;
    char dummy[12];
}
CN_GROUP;


#define CO_FILE_READ_ERROR  -1
#define CO_FILE_CHECK_ERROR -2
#define CO_FILE_READ_OK     0

#define CO_FILE_WRITE_ERROR  -1
#define CO_FILE_WRITE_DIFFILE  -2
#define CO_FILE_WRITE_OK     0

//==============================================================================
//==============================================================================
//================================================================================
extern void coInit();
extern bool coMd5(const TCHAR *path,void *md5,int md5len);
extern int32 coParse(const TCHAR *path, S_CO *co, unsigned int *offset);
extern int32 coSave(S_CO *co, TCHAR *path);
extern void coRelease(S_CO *co);
extern void coCreateIndex(S_CO_RUN *co_run, S_CO *co);
extern void coRun(S_CO_RUN *co_run);
extern uint32 corunReadLine(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size);
extern void corunRollStep(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size);
extern uint32 corunReadStep(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size);
//extern bool corunSeekLine(S_CO_RUN *co_run, uint32 line  ,uint32 size);
extern void corunReset(S_CO_RUN *co_run);
extern void coRelease(S_CO *co);

extern bool cnCreate(const TCHAR *path, S_CN_GROUP *co,unsigned int num);
extern int cnParse(const TCHAR *path, S_CN_GROUP *val);

#endif
