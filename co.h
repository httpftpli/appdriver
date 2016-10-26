#ifndef __CO__H__
#define __CO__H__


#include "type.h"
#include "list.h"
#include "ff.h"
#include "co.h"
#include "ffconf.h"


#define FEED_NUMBER   6

#define SEL_BASE   0
#define SEL_LINE_NUMBER   32
#define HAFUZHEN_NUMBER   32
#define HAFUZHEN_BASE     (SEL_BASE + FEED_NUMBER * SEL_LINE_NUMBER)
#define CAM_BASE          (HAFUZHEN_BASE + HAFUZHEN_NUMBER)
#define CAM_LINE_NUMBER   16
#define YARN_FINGER_BASE  (CAM_BASE + FEED_NUMBER*CAM_LINE_NUMBER)
#define YARN_LINE_NUMBER  16
#define VALVE_MIS_BASE    (YARN_FINGER_BASE + FEED_NUMBER*YARN_LINE_NUMBER)
#define VALVE_MIS_NUMBER  224
#define VALVE_0603_BASE   (VALVE_MIS_BASE + VALVE_MIS_NUMBER)
#define VALVE_1F01_BASE   VALVE_0603_BASE+ 32


#define COMM_FUNC_BASE    0x2c0






typedef __packed struct {
    uint16 unkown;
    char name[4];
    uint16 offset;
    uint16 len;
    uint16 decryptedLen;
    char unkown2[15];
    uint32 check;
}
CO_SECTION;


typedef __packed struct {
    uint32 coCheck;
    char coFileName[11];
    char machineName[11];
    int32 softver;
    char unkown[44];
    CO_SECTION sec[14];
    char unkown1[4];
}
CO_HEADER;


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
}
CO_RESET_INFO;



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
}
CO_PART2_ATTRIB;


__packed typedef struct {
    char unknow[40];
    uint32 func_addr;
    char unknow1[8];
    uint32 speed_addr;
    char unknow2[8];
}
CO_CATE_INFO;


__packed typedef struct {
    char unkown[56];
    uint32 fengmen1addr;
    uint32 fengmen2addr;
}
CO_MPP_INFO;


__packed typedef struct {
    uint16 beginFlag; //0xffff
    uint16 angle;
    uint16 beginStep;
    uint16 endStep;
    uint16 groupNum;
    uint16 unkown;  //0x0000
}
CO_MOTORHEAD;


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
    uint16 end[3];        //cylinder duplicate 3 times
    uint16 endWidth;
    uint16 endWidthDec; //dec: 0-0,1-0.25 3-0.75
}
CO_SIZEMOTORPARAM;


typedef struct {
    int32 start;
    uint16 startWidth;
    uint16 startWidthDec; //dec: 0-0,1-0.25 3-0.75
    int32 end;
    uint16 endWidth;
    uint16 endWidthDec; //dec: 0-0,1-0.25 3-0.75
    int32 acc;
}SIZEMOTORPARAM;

__packed typedef struct {
    unsigned int size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    CO_MOTORHEAD head;
    CO_SIZEMOTORPARAM param[8];
}
CO_SIZEMOTOR_ZONE;


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
}
CO_SINKERMOTORPARAM;

__packed typedef struct {
    uint32 size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    CO_MOTORHEAD head;
    CO_SINKERMOTORPARAM param[8];
}
CO_SINKERMOTOR_ZONE;


typedef struct {
    uint32 qi_feed;
    uint32 qf_feed;
    int32 acc;
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
}
CO_SPEED;



typedef struct {
    int32 step;
    //int32 prerpm[8];
    //int32 acc[8];
    int32 rpm;
    int16 ramp[8];
    bool coed;
    struct list_head list;
}SPEED;


__packed typedef struct {
    uint16 angular;
    uint16 value;
    uint16 funcode;
    uint8 add[4];
}
CO_FUNC;



typedef struct {
    uint16 angular;
    uint16 value;
    uint16 funcode;
    uint8 add[4];
    struct list_head list;
}FUNC;




__packed typedef struct {
    uint16 ryoyo;
    uint16 zyoyo;
    uint16 yyoyo;
    uint16 dummy[3];
    uint16 series;
}
MOTOR_HEADER_PARAM;


typedef struct {
    uint16 angular;
    uint16 dummy;
    uint16 funcode;
    uint16 val;
}CO_FENGMEN;


typedef struct {
    uint16 angular;
    uint16 dummy;
    uint16 funcode;
    uint16 val;
    struct list_head list;
}MPP;



__packed typedef struct {
    uint16 begin;
    uint16 end;
    uint16 economize[8][2];
}
CO_ECONOMIZER_PARAM;


typedef struct {
    uint16 begin;
    uint16 end;
    uint16 economize[8];
}ECONOMIZER_PARAM;




__packed typedef struct {
    uint32 flag;
    uint32 unkown;
    char headmppctrl;
    char dummy[3];
    uint32 unkown1[4];
    uint32 filedescOffset;
    uint32 weltAddrOffset;
    uint32 econoAddrOffset;
    uint32 unkown3;
    uint32 supesize;
    char unkown4[16];
}
CO_SUPE_INFO;



__packed typedef struct {
    uint16 weltflag;
    uint16 unknow;
    uint16 step;
}
CO_WELT_PARAM;

typedef struct {
    uint16 weltinstep;
    uint16 weltoutstep;
    struct list_head list;
}WELT_PARAM;


#define CO_RUN_FILENAME(CO_RUN)  ((CO_RUN)->co->filename);


typedef struct {
    wchar name[50];
    int num;
    int point;
    int numofline;
    uint32 coCheck;
    uint32 dataAvailable;
    char *data;
    int cosize;
    int datapointer;
    int ref;
    struct list_head list;
}BTSR;


/////////dis///////////////////////////////////

#define DIS_INFO_NUM_MAX          100



typedef struct {
    unsigned int diswidth;
    unsigned int dumy[7];
    unsigned int selInfoAddr;
    unsigned int selDataAddr;
    unsigned int guidInfoAddr;
    unsigned int guidDataAddr;
    unsigned char xxx[16];
}DISHEAD;


typedef struct {
    unsigned int beginNiddle;
    unsigned int endNiddle;
    unsigned int distype;
    unsigned int num;
}DISINFO;


typedef struct {
    unsigned int filetype;
    unsigned int dummy[7];
    unsigned int addrBegin;
    unsigned int xx[7];
}JACKHEAD ,GUIDHEAD;


typedef struct {
    unsigned int isel;
    unsigned int selType;
    unsigned int dummy[2];
    unsigned int xxx;
    unsigned int inNiddle;
    unsigned int xx[2];
    unsigned int xxxx;
    unsigned int outStep;
    unsigned int outNiddle;
    unsigned int disinfoAddr;
}CO_JACPAT;


typedef struct {
    unsigned int width;
    unsigned int data[1];
}CO_DIS_DATA;


/*typedef struct{
    CO_JACPAT *co_jac;
    DISINFO *disinfo;
}JACPAT;


typedef struct {
    int num;
    unsigned int bitmap;
    JACPAT pat[4];
}STEP_JAC;*/


typedef struct {
    CO_JACPAT *co_jac;
    uint32 step;
    DISINFO *disinfo;
    //struct list_head list;
}SEL_JAC;



typedef struct {
    uint32 ifeed;
    uint32 xxx[5];
    uint32 inNiddle;
    uint32 xxinNiddle;
    uint32 xx[4];
    uint32 outStep;
    uint32 outNiddle;
    uint32 xxoutNiddle;
    uint32 addr;
}CO_GUID;


typedef struct {
    CO_GUID *co_guid;
    uint16 ifeed;
    uint16 type;
    uint32 step;
    uint32 disaddr;
    uint32 lineNum;
}SEL_GUID;


typedef struct __S_CO_RUN S_CO_RUN;
typedef struct __machine_str_tag MACHINE;

typedef struct {
    wchar filename[100];
    unsigned char parsed;
    CO_HEADER head;
    uint32 diameter;
    uint16 niddle;
    uint32 numofstep;

    CO_RESET_INFO resetinfo;
    CO_CATE_INFO cateinfo;
    CO_SUPE_INFO supeinfo;

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
    MACHINE *machine;
    //dis
    unsigned int dissize;
    void *dis;
    unsigned int jacsize;
    void *jac;
    unsigned int guidsize;
    void *guid;
    //sink sinkermotor
}S_CO;


#define JACQTYPE_PAT    0x0a
#define JACQTYPE_SLZ    0x0d
#define JACQTYPE_SUP1   0x13
#define JACQTYPE_SUP0   0x12
#define JACQTYPE_GUSSET0 0x15
#define JACQTYPE_GUSSET1 0x16
#define JACQTYPE_GUSSET2 0x17


typedef struct {
    uint32 istep;
    uint32 ilinetag[8];
    //uint32 ilineEtag[8];
    uint16 econoFlag;
    ECONOMIZER_PARAM *econo;

    bool welt;

    SPEED *speed;

    SIZEMOTOR_ZONE *sizemotor;
    SINKERMOTOR_ZONE *sinkmoterzone_1_3;
    SINKERMOTOR_ZONE *sinkmoterzone_2_4;
    SINKERMOTOR_ZONE *sinkmoterzone_angle;
#define FEED_NUM      4

//#define SEL_PRI_PAT      2
//#define SEL_PRI_SUPPAT   1
//#define SEL_PRI_SLZ      0
#define SEL_PRI_NUM    6
    uint8 haveSel;
    uint32 jacsnum[FEED_NUM];
    SEL_JAC *jacs[FEED_NUM][SEL_PRI_NUM];
    uint32 guidNum;
    SEL_GUID *guid[16];
    //SEL_GUID   *cam[FEED_NUM];
    //struct list_head jacs[FEED_NUM];
    struct list_head *func;
    struct list_head *fengmen;
    struct list_head list;
}S_CO_RUN_STEP;


#define ECONO_INECONO 1
#define ECONO_BEGIN   3
#define ECONO_END     5
#define IS_ECONO_BEGIN(A)   (((A).econoFlag & ECONO_BEGIN)==ECONO_BEGIN)
#define IS_ECONO_END(A)   (((A).econoFlag & ECONO_END)==ECONO_END)
#define IS_ECONO_BEGIN_END(A)  (((A).econoFlag & (ECONO_BEGIN | ECONO_END))==(ECONO_BEGIN | ECONO_END))
#define IS_ECONO_INECONO(A)   ((A).econoFlag & ECONO_INECONO)


typedef struct {
    uint16 num;
    uint16 valvecode[64];
}ACT_GROUP;


typedef struct {
    uint16 num;
    uint16 alarmcode[15];
}ALARM_GROUP;



struct __S_CO_RUN {
    //  int32 istep;                   //step conter when run;
    //  uint32 nextline;                //下一行
    //  int32 nextstep;                //nextstep != istep+1, due to economizer
    //  int32 prestep;

    //int16 speedAcc;                 //speed acceleration when run
    //uint16 targetSpeed;             //target speed when ramp

    //bool welt;                      //is_or_not in welt flag

    //int32 sizemotor;
    // int32 stepSizemotorBase;
    //int32 stepSizemotorAcc;
    //int32 sinkmotor1_3;
    //int32 stepSinkermotor1_3Base;
    //int32 stepsinkermotor1_3Acc;
    //int32 sinkmotor2_4;
    //int32 stepsinkermotor2_4Base;
    //int32 stepsinkermotor2_4Acc;
    // FENGMEN *fengmen[360];

    //uint16 iecono;                  //current economizer counter ,begin from 1
    //uint16 econonum;                //economizer
    //uint16 econostepfrom;           //economizer begin step
    // uint16 econostepto;             //economizer end step(end>begin)
    //
    uint16 numofline[8];            //number of line ,>=numofstep due to economizer   总圈数
    uint16 numofstep;               //number offset step ,equal to CO::numofstep		总步数
    S_CO *co;                         //poit to associated co
    S_CO_RUN_STEP *stepptr[200];      //point to S_CO_RUN_STEP list element
    struct list_head step;          //CO_RUN_STEP list
    BTSR *btsr;
    //dis
    uint32 seljacnum;
    SEL_JAC *seljac[500];
    uint32 selguidnum;
    SEL_GUID *selguid[100];
};




#define LINE_FLAG_ACT 0x02
#define LINE_FLAG_SEL 0x100
#define LINE_FLAG_SEL_GUID   0x200


#define COMMON_FUNCODE_4_CODE  0x00
#define COMMON_FUNCODE_8_CODE  0x01
#define COMMON_FUNCODE_11_CODE    0x10
#define COMMON_FUNCODE_12_11_CODE 0x11
#define COMMON_FUNCODE_12_12_CODE 0x12
#define COMMON_FUNCODE_12_13_CODE 0x13
#define COMMON_FUNCODE_12_14_CODE 0x14
#define COMMON_FUNCODE_12_2c_CODE 0x2c
#define COMMON_FUNCODE_12_2d_CODE 0x2d


typedef struct {
    S_CO_RUN *co_run;

    int16 istep;                    //当前STEP
                                    //int32 nextstep;                //nextstep != istep+1, due to economizer

    int32 iline;                    //实际当前圈数
                                    //int32 nextline;                //下一行


    uint16 prerpm;
    int32 rpm;                      //当前圈设定速度
    int16 speedAcc;
    uint16 targetSpeed;             //target speed when ramp

    bool welt;                      //flag is_or_not in welt;

    uint16 iecono;                  //当前循环
    uint16 econobegin;              //当前循环首,如果没有循环为0
    uint16 econoend;                //当前循环尾,如果没有循环为0
    uint16 econonum;                //当前循环总共循环

    bool isfengmenAct;
    ACT_GROUP act[360];
    ALARM_GROUP alarm[360];
    MPP *fengmen[360];

    char *zonename;
    uint16 zonebegin;
    uint16 zoneend;

    uint32 sizemotor;                 //步进电机值


    uint32 sinkmotor1_3;               //sinker motor


    uint32 sinkmotor2_4;

    uint32 sinkmotor_angle; 

    uint32 flag;
}S_CO_RUN_LINE;


typedef struct {
    char filename[12];
    uint32 num;
}S_CN_GROUP;


__packed typedef struct {
    char co[12];
    short unknow;
    uint32 product;
    char dummy[12];
}
CN_GROUP;



struct __machine_str_tag {
    char name[20];
    uint32 niddleNum;
    uint32 feedNum;
    uint32 selPreNiddleNum;
    bool niddleNumCheck;
    void (*fun0203Resolve)(uint16 codevalue, uint16 *valvecode, uint32 *valnum);
    void (*fun031eToValvecode)(FUNC *fun, uint16 *valvecode, uint32 *valnum,
                               uint16 *alarmcode, uint32 *alarmnum);
    void (*funcode2Alarm)(FUNC *func, uint16 *alarmcode, uint32 *alarmnum);
    void (*funCamResolve)(FUNC *fun, uint16 *valvecode, uint32 *num);
    void (*fun0309CamResolve)(FUNC *fun, uint16 *valvecode, uint32 *num);
    // SONTONI Knitting Machine
    void (*fun02000220Resolve)(FUNC *fun, uint16 *valvecode, uint32 *num); //spec func and yarn finger 
};


#define CO_MACHINE_NOT_MATCH  -6
#define CO_NIDDLE_NOT_MATCH   -5
#define CO_PARSE_PARSED_ERROR  -4
#define CO_FILE_PARSE_ERROR -3
#define CO_FILE_READ_ERROR  -1
#define CO_FILE_CHECK_ERROR -2
#define CO_FILE_READ_OK     0

#define CO_FILE_WRITE_ERROR  -1
#define CO_FILE_WRITE_DIFFILE  -2
#define CO_FILE_WRITE_OK     0


#define CO_INDEX_FLAG_HAVE_BTSR  (1<<8)




typedef struct {
    uint16 valveCode;
    const char *nickname;
    bool inout;
}QIFA_ACT;


//==============================================================================
//==============================================================================
//================================================================================
extern void coInit(char machinename[], uint32 niddleNum, uint32 sel_PreNiddleNum);
extern bool coMd5(const TCHAR *path, void *md5, int md5len);
extern int32 coParse(const TCHAR *path, S_CO *co, uint32 flag, unsigned int *offset);
extern int32 coSave(S_CO *co, TCHAR *path);
extern void coRelease(S_CO *co);
extern void coCreateIndex(S_CO_RUN *co_run, S_CO *co);
extern void coRun(S_CO_RUN *co_run);
extern int32 corunReadLine(S_CO_RUN *co_run, S_CO_RUN_LINE *line, const S_CO_RUN_LINE *linepre, uint32 size);
extern void corunRollStep(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size);
extern uint32 corunReadStep(S_CO_RUN *co_run, S_CO_RUN_LINE *line, const S_CO_RUN_LINE *linepre, uint32 size);
//extern bool corunSeekLine(S_CO_RUN *co_run, uint32 line  ,uint32 size);
extern void corunReset(S_CO_RUN *co_run, S_CO_RUN_LINE *line);
extern void coRelease(S_CO *co);
extern uint16 coRunReadJacq(S_CO_RUN *co_run, S_CO_RUN_LINE *run_line, unsigned int niddle, unsigned int cosize);
extern uint32 corunReadDisfingerCam(S_CO_RUN *co_run, S_CO_RUN_LINE *run_line, unsigned int niddle, unsigned int cosize, uint16 valveCode[], uint32 valveNum);
#define CN_OK  0
#define CN_READ_ERROR    -1
#define CN_FILE_ERROR    -2
extern bool cnCreate(const TCHAR *path, S_CN_GROUP *co, unsigned int num);
extern int cnParse(const TCHAR *path, S_CN_GROUP *val);
extern void coRunInitBtsr(S_CO_RUN *co_run, int numofBtsr, int numofpoint, int co_size);
extern void coRunBtsrBeginStudy(S_CO_RUN *co_run, uint32 line);
extern void coRunBtsrStudy(S_CO_RUN *co_run, void *buf, int size);
extern bool coRunBtsrSave(S_CO_RUN *co_run);
extern bool coRunBtsrData(S_CO_RUN *co_run, int32 iline, void **data, uint32 *datasize);
extern bool coRunIsBtsrDataAvailable(S_CO_RUN *co_run);


/******************CO TEST*************************/
#define CO_TEST_FLAG_FUNC               0x01
#define CO_TEST_FLAG_JACQ               0x02
#define CO_TEST_FLAG_GUID_FIGNER        0x04
#define CO_TEST_FLAG_GUID_CAM           0x08
#define CO_TEST_FLAG_MOTOR              0x10

extern bool coActTestBegin(const TCHAR *filepath, S_CO_RUN *co);
extern void coActTest(S_CO_RUN_LINE *line);
extern void coActTestEnd();
extern void coTest(TCHAR *path, unsigned int flag);

#endif
