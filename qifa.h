
#ifndef __QIFA__H__
#define __QIFA__H__

#include <stdbool.h>
#include <wchar.h>
#include "type.h"
#include "algorithm.h"



#define Yinse_Lie   48
#define NUd 0xffff
//+++++++++++++++++++++++++++++++++++++++
#define SLfd1       0x0
#define SLfd1b      0x1
#define SLfd2       0x2
#define SLfd2b      0x3
#define SLfd3       0x4
#define SLfd3b      0x5
#define SLfd4       0x6
#define SLfd4b      0x7
#define SLfd5       0x8
#define SLfd5b      0x9
#define SLfd6       0xa
#define SLfd6b      0xb
//++++++++++++++++++++++++++++++++++++++++
#define HfCam       0xc
#define HfCamb      0xd

#define Camfd1      0xe
#define Camfd2      0xf
#define Camfd3      0x10
#define Camfd4      0x11
#define Camfd5      0x12
#define Camfd6      0x13

#define Yarnfd1      0x14
#define Yarnfd2      0x15
#define Yarnfd3      0x16
#define Yarnfd4      0x17
#define Yarnfd5      0x18
#define Yarnfd6      0x19
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SL1_1   NUd
#define SL2_1   NUd
#define SL3_1   NUd
#define SL4_1   NUd
#define SL5_1   0   //0x0508
#define SL6_1   1   //0x0401
#define SL7_1   2   //0x0400
#define SL8_1   3   //0x0509
#define SL9_1   4   //0x0607
#define SL10_1  5   //0x0606
#define SL11_1  6   //0x0605
#define SL12_1  7   //0x0604
#define SL13_1  8   //0x0603
#define SL14_1  9   //0x0602
#define SL15_1  10  //0x0601
#define SL16_1  11  //0x0600
//const unsigned short Yinse_SLfd1[16] = { SL1_1, SL2_1, SL3_1, SL4_1, SL5_1, SL6_1, SL7_1, SL8_1, SL9_1, SL10_1, SL11_1, SL12_1, SL13_1, SL14_1, SL15_1, SL16_1 };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SL1_2 NUd
#define SL2_2 NUd
#define SL3_2 NUd
#define SL4_2 NUd
#define SL5_2 NUd
#define SL6_2 NUd
#define SL7_2 NUd
#define SL8_2 NUd
#define SL9_2   12  //0x108
#define SL10_2  13  //0x109
#define SL11_2  14  //0x10a
#define SL12_2  15  //0x10b
#define SL13_2  16  //0x10c
#define SL14_2  17  //0x10d
#define SL15_2  18  //0x10e
#define SL16_2  19  //0x10f
//const unsigned short Yinse_SLfd2[16] = { SL1_2, SL2_2, SL3_2, SL4_2, SL5_2, SL6_2, SL7_2, SL8_2, SL9_2, SL10_2, SL11_2, SL12_2, SL13_2, SL14_2, SL15_2, SL16_2 };
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SL1_3   NUd
#define SL2_3   NUd
#define SL3_3   NUd
#define SL4_3   NUd
#define SL5_3   20  //0x207
#define SL6_3   21  //0x30e
#define SL7_3   22  //0x30f
#define SL8_3   23  //0x50a
#define SL9_3   24  //0x100
#define SL10_3  25  //0x101
#define SL11_3  26  //0x102
#define SL12_3  27  //0x103
#define SL13_3  28  //0x104
#define SL14_3  29  //0x105
#define SL15_3  30  //0x106
#define SL16_3  31  //0x107

//const unsigned short Yinse_SLfd3[16] = { SL1_3, SL2_3, SL3_3, SL4_3, SL5_3, SL6_3, SL7_3, SL8_3, SL9_3, SL10_3, SL11_3, SL12_3, SL13_3, SL14_3, SL15_3, SL16_3 };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define SL1_4   NUd
#define SL2_4   NUd
#define SL3_4   NUd
#define SL4_4   NUd
#define SL5_4   NUd
#define SL6_4   NUd
#define SL7_4   NUd
#define SL8_4   NUd
#define SL9_4   32  //0x60f
#define SL10_4  33  //0x60e
#define SL11_4  34  //0x60d
#define SL12_4  35  //0x60c
#define SL13_4  36  //0x60b
#define SL14_4  37  //0x60a
#define SL15_4  38  //0x609
#define SL16_4  39  //0x608


//const unsigned short Yinse_SLfd4[16] = { SL1_4, SL2_4, SL3_4, SL4_4, SL5_4, SL6_4, SL7_4, SL8_4, SL9_4, SL10_4, SL11_4, SL12_4, SL13_4, SL14_4, SL15_4, SL16_4 };
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define EV49_Hf2In  40  //0x03      //EV49哈夫针2半位    收
#define EV48_Hf1In  41  //0x02      //EV48蛤夫针1半位    收
#define EV47_Hf2Out 42  //0x01      //EV47蛤夫针2全位    出
#define EV46_Hf1Out 43  //0x00      //EV46蛤夫针1全位    出
#define Hf5_1 NUd
#define Hf6_1 NUd
#define Hf7_1 NUd
#define Hf8_1 NUd
#define Hf9_1 NUd
#define Hf10_1 NUd
#define Hf11_1 NUd
#define Hf12_1 NUd
#define Hf13_1 NUd
#define Hf14_1 NUd
#define Hf15_1 NUd
#define Hf16_1 NUd

//const unsigned short Yinse_Hffd1[16] = { Hf2_In, Hf1_In, Hf2_Out, Hf1_Out, Hf5_1, Hf6_1, Hf7_1, Hf8_1, Hf9_1, Hf10_1, Hf11_1, Hf12_1, Hf13_1, Hf14_1, Hf15_1, Hf16_1 };
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define Cam1SC 44//0X40F
#define Cam1SE 45//0X40E
#define Cam1XC NUd
#define Cam1XE NUd
#define Cam1TC 46//0X40D
#define Cam1TE 47//0X40C
//const unsigned short Yinse_Cam1[16] = { Cam1SC, Cam1SE, Cam1XC, Cam1XE, Cam1TC, Cam1TE, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define Cam2SC 48   //0X304
#define Cam2SE 49   //0X305
#define Cam2XC NUd
#define Cam2XE 50   //0X309
#define Cam2TC 51   //0X306
#define Cam2TE 52   //0X307
//const unsigned short Yinse_Cam2[16] = { Cam2SC, Cam2SE, Cam2XC, Cam2XE, Cam2TC, Cam2TE, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define Cam3SC 53//0X300
#define Cam3SE 54//0X301
#define Cam3XC NUd
#define Cam3XE NUd
#define Cam3TC 55//0X302
#define Cam3TE 56//0X303
//const unsigned short Yinse_Cam3[16] = { Cam3SC, Cam3SE, Cam3XC, Cam3XE, Cam3TC, Cam3TE, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define Cam4SC 57//0X40B
#define Cam4SE 58//0X40A
#define Cam4XC NUd
#define Cam4XE 59//0X406
#define Cam4TC 60//0X409
#define Cam4TE 61//0X408
//const unsigned short Yinse_Cam4[16] = { Cam4SC, Cam4SE, Cam4XC, Cam4XE, Cam4TC, Cam4TE, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define Gu1_1 62    //0X0507
#define Gu2_1 63//0X0506
#define Gu3_1 64//0X0505
#define Gu4_1 65//0x0504
#define Gu5_1 66//0x0503
#define Gu6_1 67//0x0502
#define Gu1v1 68//0X0501       //  1路1梭半位
#define Gu3v1 69//0X0500       //  1路3梭半位
//const unsigned short Yinse_Yarn1[16] = { Yarn1_1, Yarn2_1, Yarn3_1, Yarn4_1, Yarn5_1, Yarn6_1, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define Gu1_2 70//    0X0208
#define Gu2_2 71//    0X0209
#define Gu3_2 72//    0X020a
#define Gu4_2 73//    0x020b
#define Gu5_2 74//    0x020c
#define Gu6_2 75//    0x020d
#define Gu1v2 76//    0X020e       //  2路1梭半位
#define Gu3v2 77//    0X020f       //  2路3梭半位

//const unsigned short Yinse_Yarn2[16] = { Yarn1_2, Yarn2_2, Yarn3_2, Yarn4_2, Yarn5_2, Yarn6_2, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define Gu1_3 78//    0X0008
#define Gu2_3 79//    0X0009
#define Gu3_3 80//    0X000a
#define Gu4_3 81//    0X000b
#define Gu5_3 82//    0X000c
#define Gu6_3 83//    0X000d
#define Gu1v3 84//    0X000e       // 3路1梭半位
#define Gu3v3 85//    0X000f       // 3路3梭半位

//const unsigned short Yinse_Yarn3[16] = { Yarn1_3, Yarn2_3, Yarn3_3, Yarn4_3, Yarn5_3, Yarn6_3, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define Gu1_4 86//    0X0707
#define Gu2_4 87//    0X0706
#define Gu3_4 88//    0X0705
#define Gu4_4 89//    0X0704
#define Gu5_4 90//    0X0703
#define Gu6_4 91//    0X0702
#define Gu1v4 92//    0X0701       //4路1梭半位
#define Gu3v4 93//    0X0700       //4路3梭半位

//const unsigned short Yinse_Yarn4[16] = { Yarn1_4, Yarn2_4, Yarn3_4, Yarn4_4, Yarn5_4, Yarn6_4, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++0x1A0+++++++++++++++++++++++++++++
#define EV5     94//0x402               //0x0
#define EV17    95//0x30d               //0x1
#define EV22    96//0x30c               //0x2
#define EV34    97//0x403               //0x3
#define EV35    98//0X70B               //0x4
#define EV36    99//0x07                //0x5
#define EV38    100//0X50F               //0x6
#define EV39    101//0x404               //0x7
#define EV40    102//0x30b               //0x8
#define EV41    103//0x30a               //0x9
#define EV42    104//0x405               //0xa
#define EV43    105//0x206               //0xb
#define EV50    106//0X70F               //0xc
#define EV51    107//0x04                //0xd
#define EV52    108//0X70E               //0xe
#define EV53    NUd                 //0xf
//const unsigned short yingse_spe1[16] ={EV5,EV17,EV22,EV34,EV35,EV36,EV38,EV39,EV40,EV41,EV42,EV43,EV50,EV51,EV52,EV53};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X1B0
#define EV54    109//0x407               //0X00
#define EV56    110//0x308               //0X01
#define EV58    111//0x204               //0X02
#define EV59    112//0x50d               //0X03
#define EV60    113//0x205               //0X04
#define EV67    114//0X70D               //0X05
#define EV68    115//0X70C               //0X06
#define EV71    NUd                 //0X07
#define EV72    NUd                 //0X08
#define EV83    NUd                 //0X09
#define EV86    116//0X709               //0X0A
#define EV87    117//0x06                //0X0B
#define EV89    NUd                 //0X0C
#define EV90    NUd                 //0X0D
#define EV91    NUd                 //0X0E
#define EV92    NUd                 //0X0F
//const unsigned short yingse_spe2[16] ={EV54,EV56,EV58,EV59,EV60,EV67,EV68,EV71,EV72,EV83,EV86,EV87,EV89,EV90,EV91,EV92};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X1C0
#define EV93    NUd                 //0X00
#define EV94    NUd                 //0X01
#define EV95    NUd                 //0X02
#define EV97    NUd                 //0X03
//const unsigned short yingse_spe3[16] ={EV93,EV94,EV95,EV97,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X1D0
//const unsigned short yingse_spe4[16] ={NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X1e0
#define EV70    118//0x05           //0x0c
#define EV79    NUd                 //0x0d
#define EV80    NUd                 //0x0e
#define EV81    NUd                 //0x0f
//const unsigned short yingse_spe5[16] ={NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,EV70,EV79,EV80,EV81};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X1f0
#define EV82    NUd                 //0x00
#define EV85    119//0x0200         //0x01
                                    //0x02
#define EV96    NUd                 //0x03
#define EV101   NUd                 //0x04

#define EV107   120//0x201          //0x0a
#define EV108   NUd                 //0x0b
#define EV109   NUd                 //0x0c
#define EV110   NUd                 //0x0d
#define EV111   NUd                 //0x0e
#define EV112   NUd                 //0x0f
//const unsigned short yingse_spe6[16] ={EV82,EV85,NUd,EV96,EV101,NUd,NUd,NUd,NUd,NUd,EV107,EV108,EV109,EV110,EV111,EV112};
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X200
#define EV113   NUd                 //0x00
#define EV114   NUd                 //0x01
//const unsigned short yingse_spe7[16] ={EV113,EV114,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X210
#define dasongmoto1 NUd             //0x0e
#define dasongmoto2 NUd             //0x0f
//const unsigned short yingse_spe8[16] ={NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,dasongmoto1,dasongmoto2};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X220
#define dasongmoto3 NUd             //0x00
#define dasongmoto4 NUd             //0x01

#define EV148   NUd                 //0x0a
#define EV160   121//0x202          //0x0b
#define EV161   122//0x203          //0x0c
//const unsigned short yingse_spe9[16] ={dasongmoto3,dasongmoto4,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,EV148,EV160,EV161,NUd,NUd,NUd};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X230       yingse_spe10
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X240       yingse_spe11
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X250       yingse_spe12
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X260       yingse_spe13
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X270       yingse_spe14
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X280       0603 特殊功能码区域
#define EV45    126                 //0x00
#define EV57    127                 //0x01
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X290       0603 特殊功能码区域
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X2a0       0603 特殊功能码区域
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X2b0       0603 特殊功能码区域
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X2c0         普通功能区1  标志量输出 关断时间下位机控制
#define EV5F    123//0x50b             //0x00
#define EV6F    124//0x708             //0x01
#define EVMAN   125//0X50C             //0X02
//const unsigned short putong1[16] ={EV5F,EV6F,EVMAN,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X2d0         普通功能区2
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X2e0         普通功能区3
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X2f0         普通功能区4
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define EV84 128
//++++++++++++++++++

typedef struct
{
    wchar_t *name;                //NAME
    unsigned char board_id;     //0~10
    unsigned char xuhao;        //0~15
    unsigned short inout:1;
    unsigned short nc_no:1;
    unsigned short default_nc_no:1;
    unsigned short nc_no_change_dis:1;
    unsigned short nc_no_en:1;
    unsigned short reset_f0_inout:1;
    unsigned short cam_en:1;
    unsigned short flag:8;
}QIFA;



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

extern bool qifaInit();
extern void qifaSet(QIFA *qifa,uint32 val);
extern void qifaSet1(uint32 boardId,uint32 iqifa, uint32 val);
extern int32 qifaRead(QIFA *qifa,bool comm);
extern int32 qifaRead1(uint32 boardId,uint32 iqifa,bool comm);
extern uint32 qifaRead2(uint32 boardId, bool comm);

extern void qifaProcess();

#endif /* __QIFA__H__*/

/* __QIFA__H__*/


