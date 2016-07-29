#ifndef  __QIFACODE__H_
#define  __QIFACODE__H_


#define NUd 0xffff

#define SL1_1   0X0   //NUd
#define SL2_1   0X1   //NUd
#define SL3_1   0X2   //NUd
#define SL4_1   0X3   //NUd
#define SL5_1   0X4   //0 //0x0508
#define SL6_1   0X5//1   //0x0401
#define SL7_1   0X6//2   //0x0400
#define SL8_1   0X7//3   //0x0509
#define SL9_1   0X8//4   //0x0607
#define SL10_1  0X9//5   //0x0606
#define SL11_1  0XA//6   //0x0605
#define SL12_1  0XB//7   //0x0604
#define SL13_1  0XC//8   //0x0603
#define SL14_1  0XD//9   //0x0602
#define SL15_1  0XE//10    //0x0601
#define SL16_1  0XF//11     //0x0600


#define SL1_2   0X20    //NUd
#define SL2_2   0X21    //NUd
#define SL3_2   0X22    //NUd
#define SL4_2   0X23    //NUd
#define SL5_2   0X24    //NUd
#define SL6_2   0X25      //NUd
#define SL7_2   0X26      //NUd
#define SL8_2   0X27      //NUd
#define SL9_2   0X28    //12  //0x108
#define SL10_2  0X29    //13  //0x109
#define SL11_2  0X2A    //14  //0x10a
#define SL12_2  0X2B    //15  //0x10b
#define SL13_2  0X2C    //16  //0x10c
#define SL14_2  0X2D    //17  //0x10d
#define SL15_2  0X2E    //18  //0x10e
#define SL16_2  0X2F    //19  //0x10f

#define SL1_3   0X40//NUd
#define SL2_3   0X41//NUd
#define SL3_3   0X42//NUd
#define SL4_3   0X43//NUd
#define SL5_3   0X44//20  //0x207
#define SL6_3   0X45//21  //0x30e
#define SL7_3   0X46//22  //0x30f
#define SL8_3   0X47//23  //0x50a
#define SL9_3   0X48//24  //0x100
#define SL10_3  0X49//25  //0x101
#define SL11_3  0X4A//26  //0x102
#define SL12_3  0X4B//27  //0x103
#define SL13_3  0X4C//28  //0x104
#define SL14_3  0X4D//29  //0x105
#define SL15_3  0X4E//30  //0x106
#define SL16_3  0X4F//31  //0x107


#define SL1_4   0X60//NUd
#define SL2_4   0X61//NUd
#define SL3_4   0X62//NUd
#define SL4_4   0X63//NUd
#define SL5_4   0X64//NUd
#define SL6_4   0X65//NUd
#define SL7_4   0X66//NUd
#define SL8_4   0X67//NUd
#define SL9_4   0X68//32  //0x60f
#define SL10_4  0X69//33  //0x60e
#define SL11_4  0X6A//34  //0x60d
#define SL12_4  0X6B//35  //0x60c
#define SL13_4  0X6C//36  //0x60b
#define SL14_4  0X6D//37  //0x60a
#define SL15_4  0X6E//38  //0x609
#define SL16_4  0X6F//39  //0x608


//HAFU PIN
#define EV49_Hf2In  0XC0//40  //0x03      //EV49哈夫针2半位    收
#define EV48_Hf1In  0XC1//41  //0x02      //EV48蛤夫针1半位    收
#define EV47_Hf2Out 0XC2//42  //0x01      //EV47蛤夫针2全位    出
#define EV46_Hf1Out 0XC3//43  //0x00      //EV46蛤夫针1全位    出
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
#define Cam1SE 0XE1//45//0X40E
#define Cam1XC 0XE2//NUd
#define Cam1XE 0XE3//NUd
#define Cam1TC 0XE4//46//0X40D
#define Cam1TE 0XE5//47//0X40C
//const unsigned short Yinse_Cam1[16] = { Cam1SC, Cam1SE, Cam1XC, Cam1XE, Cam1TC, Cam1TE, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++0xF0~0xFF++++++++++++++++++++++++
#define Cam2SC 0XF0//48   //0X304
#define Cam2SE 0XF1//49   //0X305
#define Cam2XC 0XF2//NUd
#define Cam2XE 0XF3//50   //0X309
#define Cam2TC 0XF4//51   //0X306
#define Cam2TE 0XF5//52   //0X307
//const unsigned short Yinse_Cam2[16] = { Cam2SC, Cam2SE, Cam2XC, Cam2XE, Cam2TC, Cam2TE, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++0x100~0x10F++++++++++++++++++++++++
#define Cam3SC 0X100//53//0X300
#define Cam3SE 0X101//54//0X301
#define Cam3XC 0X102//NUd
#define Cam3XE 0X103//NUd
#define Cam3TC 0X104//55//0X302
#define Cam3TE 0X105//56//0X303
//const unsigned short Yinse_Cam3[16] = { Cam3SC, Cam3SE, Cam3XC, Cam3XE, Cam3TC, Cam3TE, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd, NUd };
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++0x110~0x11F++++++++++++++++++++++++
#define Cam4SC 0X110//57//0X40B
#define Cam4SE 0X111//58//0X40A
#define Cam4XC 0X112//NUd
#define Cam4XE 0X113//59//0X406
#define Cam4TC 0X114//60//0X409
#define Cam4TE 0X115//61//0X408



//YARE FINGER
#define Gu1_1 0X140//62//0X0507
#define Gu2_1 0X141//63//0X0506
#define Gu3_1 0X142//64//0X0505
#define Gu4_1 0X143//65//0x0504
#define Gu5_1 0X144//66//0x0503
#define Gu6_1 0X145//67//0x0502
#define Gu1v1 0X146//68//0X0501       //  1路1梭半位
#define Gu3v1 0X147//69//0X0500       //  1路3梭半位
#define GURF1 0X14D                   //1路1梭前移
#define GURA1 0X14F                   //1路1梭左移

#define Gu1_2 0X150//70//    0X0208
#define Gu2_2 0X151//71//    0X0209
#define Gu3_2 0X152//72//    0X020a
#define Gu4_2 0X153//73//    0x020b
#define Gu5_2 0X154//74//    0x020c
#define Gu6_2 0X155//75//    0x020d
#define Gu1v2 0X156//76//    0X020e       //  2路1梭半位
#define Gu3v2 0X157//77//    0X020f       //  2路3梭半位
#define GURF2 0X15D                   //2路1梭前移
#define GURA2 0X15F                   //2路1梭左移

#define Gu1_3 0X160//78//    0X0008
#define Gu2_3 0X161//79//    0X0009
#define Gu3_3 0X162//80//    0X000a
#define Gu4_3 0X163//81//    0X000b
#define Gu5_3 0X164//82//    0X000c
#define Gu6_3 0X165//83//    0X000d
#define Gu1v3 0X166//84//    0X000e       // 3路1梭半位
#define Gu3v3 0X167//85//    0X000f       // 3路3梭半位
#define GURF3 0X16D                   //3路1梭前移
#define GURA3 0X16F                   //3路1梭左移


#define Gu1_4 0X170//86//    0X0707
#define Gu2_4 0X171//87//    0X0706
#define Gu3_4 0X172//88//    0X0705
#define Gu4_4 0X173//89//    0X0704
#define Gu5_4 0X174//90//    0X0703
#define Gu6_4 0X175//91//    0X0702
#define Gu1v4 0X176//92//    0X0701       //4路1梭半位
#define Gu3v4 0X177//93//    0X0700       //4路3梭半位
#define GURF4 0X17D                   //4路1梭前移
#define GURA4 0X17F                   //4路1梭左移

//SP FUNCTION CODE
#define EV5     0X1A0//94//0x402               //0x0
#define EV17    0X1A1//95//0x30d               //0x1
#define EV22    0X1A2//96//0x30c               //0x2
#define EV34    0X1A3//97//0x403               //0x3
#define EV35    0X1A4//98//0X70B               //0x4
#define EV36    0X1A5//99//0x07                //0x5
#define EV38    0X1A6//100//0X50F               //0x6
#define EV39    0X1A7//101//0x404               //0x7
#define EV40    0X1A8//102//0x30b               //0x8
#define EV41    0X1A9//103//0x30a               //0x9
#define EV42    0X1AA//104//0x405               //0xa
#define EV43    0X1AB//105//0x206               //0xb
#define EV50    0X1AC//106//0X70F               //0xc
#define EV51    0X1AD//107//0x04                //0xd
#define EV52    0X1AE//108//0X70E               //0xe
#define EV53    0X1AF//NUd                 //0xf
#define EV54    0X1B0//109//0x407               //0X00
#define EV56    0X1B1//110//0x308               //0X01
#define EV58    0X1B2//111//0x204               //0X02
#define EV59    0X1B3//112//0x50d               //0X03
#define EV60    0X1B4//113//0x205               //0X04
#define EV67    0X1B5//114//0X70D               //0X05
#define EV68    0X1B6//115//0X70C               //0X06
#define EV71    0X1B7//NUd                 //0X07
#define EV72    0X1B8//NUd                 //0X08
#define EV83    0X1B9//NUd                 //0X09
#define EV86    0X1BA//116//0X709               //0X0A
#define EV87    0X1BB//117//0x06                //0X0B
#define EV89    0X1BC//NUd                 //0X0C
#define EV90    0X1BD//NUd                 //0X0D
#define EV91    0X1BE//NUd                 //0X0E
#define EV92    0X1BF//NUd                 //0X0F
#define EV93    0X1C0//NUd                 //0X00
#define EV94    0X1C1//NUd                 //0X01
#define EV95    0X1C2//NUd                 //0X02
#define EV97    0X1C3//NUd                 //0X03

#define EV70    0X1EC//118//0x05           //0x0c
#define EV79    0X1ED//NUd                 //0x0d
#define EV80    0X1EE//NUd                 //0x0e
#define EV81    0X1EF//NUd                 //0x0f
#define EV82    0X1F0//NUd                 //0x00
#define EV85    0X1F1//119//0x0200         //0x01
#define EV96    0X1F3//NUd                 //0x03
#define EV101   0X1F4//NUd                 //0x04
#define EV107   0X1FA//120//0x201          //0x0a
#define EV108   0X1FB//NUd                 //0x0b
#define EV109   0X1FC//NUd                 //0x0c
#define EV110   0X1FD//NUd                 //0x0d
#define EV111   0X1FE//NUd                 //0x0e
#define EV112   0X1FF//NUd                 //0x0f
#define EV113   0X200//NUd                 //0x00
#define EV114   0X201//NUd                 //0x01

#define dasongmoto1 0X21E   //NUd             //0x0e
#define dasongmoto2 0X21F   //NUd             //0x0f
//const unsigned short yingse_spe8[16] ={NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,NUd,dasongmoto1,dasongmoto2};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++++++++++++++++//0X220~0X22F
#define dasongmoto3 0X220//NUd             //0x00
#define dasongmoto4 0X221//NUd             //0x01

#define EV148   0X22A//NUd                 //0x0a
#define EV160   0X22B//121//0x202          //0x0b
#define EV161   0X22C//122//0x203          //0x0c
#define EV45    0X280//126                 //0x00
#define EV57    0X281//127                 //0x01
#define EV84    0X2A5                   //128


//COMM FUNCTION CODE

#define EV5F    0X2C0//123//0x50b             //0x00
#define EV6F    0X2C1//124//0x708             //0x01
#define EVMAN   0X2C2//125//0X50C             //0X02



#endif

