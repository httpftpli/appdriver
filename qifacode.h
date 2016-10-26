//#ifndef  __QIFACODE__H_
//#define  __QIFACODE__H_

#ifndef ACT 
#define ACT(A)
#endif

#define NUd 0xffff

#define SL1_1   0X0   //NUd
ACT(SL1_1) 
#define SL2_1   0X1   //NUd
ACT(SL2_1)                      
#define SL3_1   0X2   //NUd
ACT(SL3_1)                      
#define SL4_1   0X3   //NUd
ACT(SL4_1)
#define SL5_1   0X4   //0 //0x0508
ACT(SL5_1)
#define SL6_1   0X5//1   //0x0401
ACT(SL6_1)
#define SL7_1   0X6//2   //0x0400
ACT(SL7_1)
#define SL8_1   0X7//3   //0x0509
ACT(SL8_1)
#define SL9_1   0X8//4   //0x0607
ACT(SL9_1)
#define SL10_1  0X9//5   //0x0606
ACT(SL10_1)
#define SL11_1  0XA//6   //0x0605
ACT(SL11_1)
#define SL12_1  0XB//7   //0x0604
ACT(SL12_1)
#define SL13_1  0XC//8   //0x0603
ACT(SL13_1)
#define SL14_1  0XD//9   //0x0602
ACT(SL14_1)
#define SL15_1  0XE//10  //0x0601
ACT(SL15_1)
#define SL16_1  0XF//11  //0x0600
ACT(SL16_1)


#define SL1_2   0X20    //NUd
ACT(SL1_2)
#define SL2_2   0X21    //NUd
ACT(SL2_2)
#define SL3_2   0X22    //NUd
ACT(SL3_2)
#define SL4_2   0X23    //NUd
ACT(SL4_2)
#define SL5_2   0X24    //NUd
ACT(SL5_2)
#define SL6_2   0X25      //NUd
ACT(SL6_2)
#define SL7_2   0X26      //NUd
ACT(SL7_2)
#define SL8_2   0X27      //NUd
ACT(SL8_2)
#define SL9_2   0X28    //12  //0x108
ACT(SL9_2)
#define SL10_2  0X29    //13  //0x109
ACT(SL10_2)
#define SL11_2  0X2A    //14  //0x10a
ACT(SL11_2)
#define SL12_2  0X2B    //15  //0x10b
ACT(SL12_2)
#define SL13_2  0X2C    //16  //0x10c
ACT(SL13_2)
#define SL14_2  0X2D    //17  //0x10d
ACT(SL14_2)
#define SL15_2  0X2E    //18  //0x10e
ACT(SL15_2)
#define SL16_2  0X2F    //19  //0x10f
ACT(SL16_2)

#define SL1_3   0X40//NUd
ACT(SL1_3)
#define SL2_3   0X41//NUd
ACT(SL2_3)
#define SL3_3   0X42//NUd
ACT(SL3_3)
#define SL4_3   0X43//NUd
ACT(SL4_3)
#define SL5_3   0X44//20  //0x207
ACT(SL5_3)
#define SL6_3   0X45//21  //0x30e
ACT(SL6_3)
#define SL7_3   0X46//22  //0x30f
ACT(SL7_3)
#define SL8_3   0X47//23  //0x50a
ACT(SL8_3)
#define SL9_3   0X48//24  //0x100
ACT(SL9_3)
#define SL10_3  0X49//25  //0x101
ACT(SL10_3)
#define SL11_3  0X4A//26  //0x102
ACT(SL11_3)
#define SL12_3  0X4B//27  //0x103
ACT(SL12_3)
#define SL13_3  0X4C//28  //0x104
ACT(SL13_3)
#define SL14_3  0X4D//29  //0x105
ACT(SL14_3)
#define SL15_3  0X4E//30  //0x106
ACT(SL15_3)
#define SL16_3  0X4F//31  //0x107
ACT(SL16_3)


#define SL1_4   0X60//NUd
ACT(SL1_4)
#define SL2_4   0X61//NUd
ACT(SL2_4)
#define SL3_4   0X62//NUd
ACT(SL3_4)
#define SL4_4   0X63//NUd
ACT(SL4_4)
#define SL5_4   0X64//NUd
ACT(SL5_4)
#define SL6_4   0X65//NUd
ACT(SL6_4)
#define SL7_4   0X66//NUd
ACT(SL7_4)
#define SL8_4   0X67//NUd
ACT(SL8_4)
#define SL9_4   0X68//32  //0x60f
ACT(SL9_4)
#define SL10_4  0X69//33  //0x60e
ACT(SL10_4)
#define SL11_4  0X6A//34  //0x60d
ACT(SL11_4)
#define SL12_4  0X6B//35  //0x60c
ACT(SL12_4)
#define SL13_4  0X6C//36  //0x60b
ACT(SL13_4)
#define SL14_4  0X6D//37  //0x60a
ACT(SL14_4)
#define SL15_4  0X6E//38  //0x609
ACT(SL15_4)
#define SL16_4  0X6F//39  //0x608
ACT(SL16_4)

//HAFU PIN
#define EV49_Hf2In  0XC0//40  //0x03      //EV49哈夫针2半位    收
ACT(EV49_Hf2In)
#define EV48_Hf1In  0XC1//41  //0x02      //EV48蛤夫针1半位    收
ACT(EV48_Hf1In)
#define EV47_Hf2Out 0XC2//42  //0x01      //EV47蛤夫针2全位    出
ACT(EV47_Hf2Out)
#define EV46_Hf1Out 0XC3//43  //0x00      //EV46蛤夫针1全位    出
ACT(EV46_Hf1Out)
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

//CAM
#define Cam1SC 0XE0//44//0X40F
ACT(Cam1SC)
#define Cam1SE 0XE1//45//0X40E
ACT(Cam1SE)
#define Cam1XC 0XE2//NUd
ACT(Cam1XC)
#define Cam1XE 0XE3//NUd
ACT(Cam1XE)
#define Cam1TC 0XE4//46//0X40D
ACT(Cam1TC)
#define Cam1TE 0XE5//47//0X40C
ACT(Cam1TE)
#define Cam2SC 0XF0//48   //0X304
ACT(Cam2SC)
#define Cam2SE 0XF1//49   //0X305
ACT(Cam2SE)
#define Cam2XC 0XF2//NUd
ACT(Cam2XC)
#define Cam2XE 0XF3//50   //0X309
ACT(Cam2XE)
#define Cam2TC 0XF4//51   //0X306
ACT(Cam2TC)
#define Cam2TE 0XF5//52   //0X307
ACT(Cam2TE)
#define Cam3SC 0X100//53//0X300
ACT(Cam3SC)
#define Cam3SE 0X101//54//0X301
ACT(Cam3SE)
#define Cam3XC 0X102//NUd
ACT(Cam3XC)
#define Cam3XE 0X103//NUd
ACT(Cam3XE)
#define Cam3TC 0X104//55//0X302
ACT(Cam3TC)
#define Cam3TE 0X105//56//0X303
ACT(Cam3TE)
#define Cam4SC 0X110//57//0X40B
ACT(Cam4SC)
#define Cam4SE 0X111//58//0X40A
ACT(Cam4SE)
#define Cam4XC 0X112//NUd
ACT(Cam4XC)
#define Cam4XE 0X113//59//0X406
ACT(Cam4XE)
#define Cam4TC 0X114//60//0X409
ACT(Cam4TC)
#define Cam4TE 0X115//61//0X408
ACT(Cam4TE)


//YARE FINGER
#define Gu1_1 0X140//62//0X0507
ACT(Gu1_1)
#define Gu2_1 0X141//63//0X0506
ACT(Gu2_1)
#define Gu3_1 0X142//64//0X0505
ACT(Gu3_1)
#define Gu4_1 0X143//65//0x0504
ACT(Gu4_1)
#define Gu5_1 0X144//66//0x0503
ACT(Gu5_1)
#define Gu6_1 0X145//67//0x0502
ACT(Gu6_1)
#define Gu1v1 0X146//68//0X0501       //  1路1梭半位
ACT(Gu1v1)
#define Gu3v1 0X147//69//0X0500       //  1路3梭半位
ACT(Gu3v1)
#define GURF1 0X14D                   //1路1梭前移
ACT(GURF1)
#define GURA1 0X14F                   //1路1梭左移
ACT(GURA1)
#define Gu1_2 0X150//70//    0X0208
ACT(Gu1_2)
#define Gu2_2 0X151//71//    0X0209
ACT(Gu2_2)
#define Gu3_2 0X152//72//    0X020a
ACT(Gu3_2)
#define Gu4_2 0X153//73//    0x020b
ACT(Gu4_2)
#define Gu5_2 0X154//74//    0x020c
ACT(Gu5_2)
#define Gu6_2 0X155//75//    0x020d
ACT(Gu6_2)
#define Gu1v2 0X156//76//    0X020e   //  2路1梭半位
ACT(Gu1v2)
#define Gu3v2 0X157//77//    0X020f   //  2路3梭半位
ACT(Gu3v2)
#define GURF2 0X15D                   //2路1梭前移
ACT(GURF2)
#define GURA2 0X15F                   //2路1梭左移
ACT(GURA2)
#define Gu1_3 0X160//78//    0X0008
ACT(Gu1_3)
#define Gu2_3 0X161//79//    0X0009
ACT(Gu2_3)
#define Gu3_3 0X162//80//    0X000a
ACT(Gu3_3)
#define Gu4_3 0X163//81//    0X000b
ACT(Gu4_3)
#define Gu5_3 0X164//82//    0X000c
ACT(Gu5_3)
#define Gu6_3 0X165//83//    0X000d
ACT(Gu6_3)
#define Gu1v3 0X166//84//    0X000e       // 3路1梭半位
ACT(Gu1v3)
#define Gu3v3 0X167//85//    0X000f       // 3路3梭半位
ACT(Gu3v3)
#define GURF3 0X16D                   //3路1梭前移
ACT(GURF3)
#define GURA3 0X16F                   //3路1梭左移
ACT(GURA3)


#define Gu1_4 0X170//86//    0X0707
ACT(Gu1_4)
#define Gu2_4 0X171//87//    0X0706
ACT(Gu2_4)
#define Gu3_4 0X172//88//    0X0705
ACT(Gu3_4)
#define Gu4_4 0X173//89//    0X0704
ACT(Gu4_4)
#define Gu5_4 0X174//90//    0X0703
ACT(Gu5_4)
#define Gu6_4 0X175//91//    0X0702
ACT(Gu6_4) 
#define Gu1v4 0X176//92//    0X0701       //4路1梭半位
ACT(Gu1v4)
#define Gu3v4 0X177//93//    0X0700       //4路3梭半位
ACT(Gu3v4)
#define GURF4 0X17D                   //4路1梭前移
ACT(GURF4)
#define GURA4 0X17F                   //4路1梭左移
ACT(GURA4)
//SP FUNCTION CODE
#define EV5     0X1A0//94//0x402               //0x0
ACT(EV5)
#define EV17    0X1A1//95//0x30d               //0x1
ACT(EV17)
#define EV22    0X1A2//96//0x30c               //0x2
ACT(EV22)
#define EV34    0X1A3//97//0x403               //0x3
ACT(EV34)
#define EV35    0X1A4//98//0X70B               //0x4
ACT(EV35)
#define EV36    0X1A5//99//0x07                //0x5
ACT(EV36)
#define EV38    0X1A6//100//0X50F               //0x6
ACT(EV38)
#define EV39    0X1A7//101//0x404               //0x7
ACT(EV39)
#define EV40    0X1A8//102//0x30b               //0x8
ACT(EV40)
#define EV41    0X1A9//103//0x30a               //0x9
ACT(EV41)
#define EV42    0X1AA//104//0x405               //0xa
ACT(EV42)
#define EV43    0X1AB//105//0x206               //0xb
ACT(EV43)
#define EV50    0X1AC//106//0X70F               //0xc
ACT(EV50)
#define EV51    0X1AD//107//0x04                //0xd
ACT(EV51)
#define EV52    0X1AE//108//0X70E               //0xe
ACT(EV52)
#define EV53    0X1AF//NUd                      //0xf
ACT(EV53)
#define EV54    0X1B0//109//0x407               //0X00
ACT(EV54)
#define EV56    0X1B1//110//0x308               //0X01
ACT(EV56)
#define EV58    0X1B2//111//0x204               //0X02
ACT(EV58)
#define EV59    0X1B3//112//0x50d               //0X03
ACT(EV59)
#define EV60    0X1B4//113//0x205               //0X04
ACT(EV60)
#define EV67    0X1B5//114//0X70D               //0X05
ACT(EV67)
#define EV68    0X1B6//115//0X70C               //0X06
ACT(EV68)
#define EV71    0X1B7//NUd                 //0X07
ACT(EV71)
#define EV72    0X1B8//NUd                 //0X08
ACT(EV72)
#define EV83    0X1B9//NUd                 //0X09
ACT(EV83)
#define EV86    0X1BA//116//0X709               //0X0A
ACT(EV86)
#define EV87    0X1BB//117//0x06                //0X0B
ACT(EV87)
#define EV89    0X1BC//NUd                 //0X0C
ACT(EV89)
#define EV90    0X1BD//NUd                 //0X0D
ACT(EV90)
#define EV91    0X1BE//NUd                 //0X0E
ACT(EV91)
#define EV92    0X1BF//NUd                 //0X0F
ACT(EV92)
#define EV93    0X1C0//NUd                 //0X00
ACT(EV93)
#define EV94    0X1C1//NUd                 //0X01
ACT(EV94)
#define EV95    0X1C2//NUd                 //0X02
ACT(EV95)
#define EV97    0X1C3//NUd                 //0X03
ACT(EV97)
#define EV70    0X1EC//118//0x05           //0x0c
ACT(EV70)
#define EV79    0X1ED//NUd                 //0x0d
ACT(EV79)
#define EV80    0X1EE//NUd                 //0x0e
ACT(EV80)
#define EV81    0X1EF//NUd                 //0x0f
ACT(EV81)
#define EV82    0X1F0//NUd                 //0x00
ACT(EV82)
#define EV85    0X1F1//119//0x0200         //0x01
ACT(EV85)
#define EV88    0X1F2                 //
ACT(EV88)
#define EV96    0X1F3//NUd                 //0x03
ACT(EV96)
#define EV101   0X1F4//NUd                 //0x04
ACT(EV101)
#define EV102   0X1F5
ACT(EV102)
#define EV103   0X1F6
ACT(EV103)
#define EV104   0X1F7
ACT(EV104)
#define EV107   0X1FA//120//0x201          //0x0a
ACT(EV107)
#define EV108   0X1FB//NUd                 //0x0b
ACT(EV108)
#define EV109   0X1FC//NUd                 //0x0c
ACT(EV109)
#define EV110   0X1FD//NUd                 //0x0d
ACT(EV110)
#define EV111   0X1FE//NUd                 //0x0e
ACT(EV111)
#define EV112   0X1FF//NUd                 //0x0f
ACT(EV112)
#define EV113   0X200//NUd                 //0x00
ACT(EV113)
#define EV114   0X201//NUd                 //0x01
ACT(EV114)
#define dasongmoto1 0X21E   //NUd             //0x0e
ACT(dasongmoto1)
#define dasongmoto2 0X21F   //NUd             //0x0f
ACT(dasongmoto2)
#define dasongmoto3 0X220//NUd             //0x00
ACT(dasongmoto3)
#define dasongmoto4 0X221//NUd             //0x01
ACT(dasongmoto4)
#define EV146   0X228//NUd                 //0x0a
ACT(EV146)
#define EV147   0X229//NUd                 //0x0a
ACT(EV147)
#define EV148   0X22A//NUd                 //0x0a
ACT(EV148)
#define EV160   0X22B//121//0x202          //0x0b
ACT(EV160)
#define EV161   0X22C//122//0x203          //0x0c
ACT(EV161)
#define EV45    0X280//126                 //0x00
ACT(EV45)
#define EV57    0X281//127                 //0x01
ACT(EV57)
#define EV84    0X2A5                   //128
ACT(EV84)

//COMM FUNCTION CODE


#define EV5F    0X2C0//123//0x50b             //0x00
ACT(EV5F)
#define EV6F    0X2C1//124//0x708             //0x01
ACT(EV6F)
#define EVMAN   0X2C2//125//0X50C             //0X02
ACT(EVMAN)

#undef ACT

//#endif

