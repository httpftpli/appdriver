#ifndef __CAN__DMP__H__
#define __CAN__DMP__H__

#include "list.h"
#include <stdbool.h>
#include "can_wp.h"
#include "ff.h"

#define DEV_TYPE_MAX_NUM   16



#define CAN_DMP_BROADCAST        0x7ffffff


#define DMP_DEV_INIT           0UL
#define DMP_DEV_DESC_FINISH    1UL
#define DMP_DEV_PRASE_FINISH   2UL
#define DMP_DEV_PRASE_ERROR    3UL
#define DMP_DEV_PARSE_MASK     0xffUL
#define DMP_DEV_DEV_ONLINE         1UL<<8
#define DMP_DEV_DEV_NEWADDED       2UL<<8
#define DMP_DEV_DEV_REMOVED        3UL<<8
#define DMP_DEV_DEV_ONLINEMASK       0xff00UL


//#define DMP_DEV_HDTYPE_SBJ    ('S'|'B'<<8|'J'<<16)
//#define DMP_DEV_HDTYPE_SSF    ('S'|'S'<<8|'F'<<16)
//#define DMP_DEV_HDTYPE_SSR    ('S'|'I'<<8|'N'<<16)
//#define DMP_DEV_HDTYPE_SSC    ('S'|'Q'<<8|'F'<<16)
//#define DMP_DEV_HDTYPE_SHB    ('S'|'H'<<8|'B'<<16)

//#define DMP_DEV_HDTYPE_SMD    ('S'|'M'<<8|'D'<<16)   //主板



#define DEV_COMP_DIFUID
#define DEV_COMP_DIFHARD

#include "list.h"



#define DMP_DEV_HEARTBEAT_STATE_WAITE    1
#define DMP_DEV_HEARTBEAT_STATE_OK       0
#define DMP_DEV_HEARTBEAT_STATE_OK_MASK  1
#define DMP_DEV_HEARTBEAT_STATE_TIMEOUT  (1<<8)


typedef struct {
    unsigned int devNum;
    unsigned int typeFlag;
    unsigned int typeIndex;
    unsigned int magic;
    wchar   name[20];
}DMP_DEVGP_PARAM;

typedef struct slave_dev DMP_DEV;
struct slave_dev {
    unsigned int stat;
    unsigned int hdtype;    //硬件类型
    unsigned char inboot;   //是否在boot
    unsigned char hdver;    //硬件版本
    unsigned char customcode[10];
    unsigned char rombigver;
    unsigned short rommidver;
    unsigned short romlitver;
    unsigned int romtime;   //时间 time_t
    unsigned int uid;       //唯一 id
    unsigned short workid;  //工作id
    unsigned char canChannel;
    unsigned int timespan;
    unsigned long long __timespan_;
    bool (*heartBeat)(DMP_DEV *dev);
    struct list_head list;
    int ipacket;
    char desc[50];
    DMP_DEVGP_PARAM *param;
};



typedef struct {
    uint32 num;
    DMP_DEV devs[32];
}DMP_IAP_HELPER;


typedef struct uid_id_ {
    unsigned int uid;
    unsigned int id;
    unsigned int hdtype;
}DMP_UID_ID;






typedef struct dmp_dev_help_ {
    struct list_head regester;
    struct list_head newadd;
    struct list_head unknow;
    DMP_DEVGP_PARAM *param;
} DMP_DEV_GROUP;



typedef struct dmp_system_ {
    unsigned char cfgmd5[16];
    unsigned int typeNum;
    DMP_DEV_GROUP dev[DEV_TYPE_MAX_NUM];
    struct list_head unknow;
    struct list_head heartbeartreturnlist;
} DMP_SYSTEM;


extern DMP_SYSTEM dmpSys;

typedef struct {
    unsigned int uid:27;
    unsigned int dir:1;
    unsigned int flag:1;
    unsigned int candir:1;
    unsigned int xtd:1;
    unsigned int dlc;
    unsigned int data[2];
}CAN_DMP;

typedef struct dev_compare_ {
    unsigned int compstat;
    struct list_head list;
}DMP_DEV_COMP;


static inline unsigned int dmpIdType(unsigned int devTypeIndex) {
    return(devTypeIndex + 2) << 6;
}

static inline bool isDevOnline(DMP_DEV *dev) {
    bool reval = (dev->stat & DMP_DEV_DEV_ONLINEMASK) == DMP_DEV_DEV_ONLINE;
    return reval;
}


static inline bool isDmpFrameRcv(CAN_DMP *frame) {
    if ((frame->flag == 0) && (frame->dir == 1)) {
        return true;
    }
    return false;
}

extern int CanDmp_Return;


extern unsigned int dmpdevtype2index(unsigned int type);
extern void dmpInit();
extern bool dmpLoadCfg(const TCHAR *path);
extern bool dmpAutoPreRegester(unsigned int devTypeIndex, unsigned int *id, DMP_DEV **newadddev);
extern void praseDevList(unsigned int canModule,CAN_DMP *frame, struct list_head *devlist);
extern void dmpCategDevice(DMP_SYSTEM *sys, struct list_head *devlist);
extern void dmpIapHelper(DMP_IAP_HELPER *helper, uint32 num);
extern int dmpSysStore();
extern bool dmpCheckDev();
extern bool dmpSysSave();
extern bool dmpAutoRegester(unsigned int devTypeIndex);
extern uint32 dmpsysListOffline(unsigned int devTypeIndex, DMP_DEV **devbuf, uint32 num);
extern bool dmpWillRegAuto(unsigned int devTypeIndex);
extern bool dmpRegester(DMP_DEV *dev, unsigned int id);
extern bool dmpDevRecoverId(DMP_DEV *dev);
extern void dmpSysDevCnt(unsigned int devtypeIndex, unsigned int *regedCnt,
                         unsigned int *offlineCnt, unsigned int *newCnt,
                         unsigned int *unknowCnt);
extern unsigned int  dmpSysTypeCnt(void);

extern unsigned int  dmpSysDevCntRequire(unsigned int devtypeIndex);
extern const wchar * dmpSysTypeName(unsigned int devtypeIndex);
extern bool dmpUnRegester1(unsigned int devTypeIndex, unsigned int workId);
extern bool dmpHaveNewAdd(unsigned int devTypeIndex);
extern bool dmpHaveOffline(unsigned int devTypeIndex);
extern DMP_DEV* dmpPreRegester1(unsigned int devTypeIndex, bool val);
extern bool dmpPreRegester2(DMP_DEV *dev, bool val);
extern void composeDev(DMP_DEV *list1, DMP_DEV *list2, DMP_DEV_COMP resultList);
extern void dmpUnregesterOffline(unsigned int devTypeIndex);
extern bool dmpWillUnReg(unsigned int devTypeIndex);
extern unsigned int dmpSysWillRegCnt(unsigned int typeindex, uint32 *flag);
extern DMP_DEV* dmpSysHeartbeat();
extern bool dmpDevHeartbeatEx(unsigned short workId);
extern void dmpSysHeartbeatEn(bool en);
extern void dmSetRegHook(unsigned int devTypeIndex, void (*hook)(DMP_DEV *));
extern int32 wpDevHeartbeatEn(DMP_DEV *dev, bool en, bool (*heartbeart)(DMP_DEV *dev), unsigned int timeoutms);




extern void dmpCanRdDevVer(unsigned int can,unsigned int uid); 
extern bool dmpCanPreSetId(unsigned int can,unsigned int devUid, bool val, unsigned int timeout);
extern bool dmpCanSetId(unsigned int can,unsigned int devUid, unsigned short id, unsigned int timeout);
extern bool dmpCanRcv(unsigned int canModule,CAN_DMP *frame);
extern bool wpHeartBeartCanRcv(CAN_WP *frame);
extern bool dmpCanReadId(unsigned int can,unsigned int devUid, unsigned short *id, unsigned int timeoutms);
extern int32 wpCanHeartbeatEn(unsigned int can,unsigned int id, bool en, unsigned int timeoutms);
extern bool praseDev(const char *desc, DMP_DEV *dev);




extern bool dmpCanJumpToBoot(unsigned int can,unsigned int devUid, unsigned int timeout);
extern bool dmpCanJumpToApp(unsigned int can,unsigned int devUid, unsigned int timeout);
extern bool dmpCanBootEraseApp(unsigned int can,unsigned int devUid, unsigned int max_baohao, unsigned int timeout);
extern bool dmpCanProgramDate(unsigned int can,unsigned int devUid, unsigned int baohao, unsigned char count, unsigned char *val, unsigned int timeout);
extern bool dmpCanProgramEnd(unsigned int can,unsigned int devUid, unsigned int timeout);
#endif /*__CAN__DMP__H__*/
