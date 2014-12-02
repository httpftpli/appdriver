
#include "pf_can.h"
#include "can_dmp.h"
#include "list.h"
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "module.h"
#include "pf_timertick.h"
#include "mmath.h"
#include "pf_bootloader.h"
#include "mmcsd_proto.h"
#include "delay.h"
#include "atomic.h"


#define DMPDEV_INAND_SECTOR  USER_SECTOR+10


#define DMP_FUNCODE_READVER        0x01
#define DMP_FUNCODE_READID         	0x02
#define DMP_FUNCODE_JUMPBOOT       0xfb
#define DMP_FUNCODE_JUMPAPP        	0xfc
#define DMP_FUNCODE_PRESETID       0xfa
#define DMP_FUNCODE_SETID               0xfd
#define DMP_FUNCODE_ERASEAPP        0xfb
#define DMP_FUNCODE_BURNAPP         0xf7



#define DEFINE_CAN_DMP_FRAME(FRAME)  CAN_DMP FRAME = {.flag = 0,.dir=0,.candir=0,.xtd=1,.dlc=1}


extern mmcsdCtrlInfo mmcsdctr[2];
DMP_SYSTEM dmpSys = { .num = { 4, 3, 10, 1 },};

static atomic setidflag,readidflag,jumptobootflag,presetidflag;
unsigned char workid;

static bool str2month(const char *str, unsigned int *month) {
    if (strlen(str) >= 4) {
        return false;
    }
    unsigned int d = *(unsigned int *)str;
    switch (d) {
    case 'J' | 'a' << 8 | 'n' << 16:
        *month = 0;
        break;
    case 'F' | 'e' << 8 | 'b' << 16:
        *month = 1;
        break;
    case 'M' | 'a' << 8 | 'r' << 16:
        *month = 2;
        break;
    case 'A' | 'p' << 8 | 'r' << 16:
        *month = 3;
        break;
    case 'M' | 'a' << 8 | 'y' << 16:
        *month = 4;
        break;
    case 'J' | 'u' << 8 | 'n' << 16:
        *month = 5;
        break;
    case 'J' | 'u' << 8 | 'l' << 16:
        *month = 6;
        break;
    case 'A' | 'u' << 8 | 'g' << 16:
        *month = 7;
        break;
    case 'S' | 'e' <<  8 | 'p' << 16:
        *month = 8;
        break;
    case 'O' | 'c' << 8 | 't' << 16:
        *month = 9;
        break;
    case 'N' | 'o' << 8 | 'v' << 16:
        *month = 10;
        break;
    case 'D' | 'e' << 8 | 'c' << 16:
        *month = 11;
        break;
    default:
        return false;
    }
    return true;
}



bool strptime(const char *str, struct tm *time) {
    char buf[50], *p;
    strncpy(buf, str, 49);
    p = strtok(buf, " ");
    unsigned int month, day, year;
    //month
    if ((p == NULL) || (!str2month(p, &month))) {
        return false;
    }
    time->tm_mon = month;
    //day
    p = strtok(NULL, " ");
    if (p == NULL) {
        return false;
    }
    day = strtol(p, NULL, 10);
    if ((day < 1) || (day > 31)) {
        return false;
    }
    time->tm_mday = day;
    //year
    p = strtok(NULL, " ");
    if (p == NULL) {
        return false;
    }
    year = strtol(p, NULL, 10);
    if (year <= 1900) {
        return false;
    }
    time->tm_year = year - 1900;
    //other
    time->tm_sec = time->tm_min = time->tm_hour = 0;
    time->tm_isdst = 0;

    return true;
}


static bool praseDev(const char *desc, DMP_DEV *dev) {
    char *ver, *date, *p;
    char buf[50];
    strncpy(buf, desc, 49);
	buf[49] = 0;
    ver = strtok(buf, "&");
    date = strtok(NULL, "&");
    if ((ver == NULL) || (date == NULL)) {
        return  false;
    }
    //prase ver
    p = strtok(ver, ".");
    if ((p == NULL) || (strlen(p) != 1))  return false;
    dev->inboot = (*p == 'A') ? 0 : 1;
    p = strtok(NULL, ".");
    if ((p == NULL) || (strlen(p) != 4))  return false;
    dev->hdtype = *((unsigned int *)p) & 0x00ffffff;
    dev->hdver = *(unsigned char *)(p + 3);
    p = strtok(NULL, ".");
	if ((p == NULL)||(strlen(p)>4)) return false;
    char ptemp[6]= {0,0,0,0,0,0};
    strncpy(ptemp,p,5);
    dev->customcode = *((unsigned int *)ptemp);
    p = strtok(NULL, ".");
    if ((p == NULL) || (strlen(p) != 2))  return false;
    dev->rombigver = (unsigned char)strtol(p, NULL, 10);
    p = strtok(NULL, ".");
    if ((p == NULL) || (strlen(p) != 2))  return false;
    dev->rommidver = (unsigned char)strtol(p, NULL, 10);
    p = strtok(NULL, ".");
    if ((p == NULL) || (strlen(p) != 2))  return false;
    dev->romlitver = (unsigned char)strtol(p, NULL, 10);

    //prase time
    struct tm  tm_time;
    if (!strptime(date, &tm_time)) {
        return false;
    }
    dev->romtime = mktime(&tm_time);
    dev->stat = dev->stat & DMP_DEV_PARSE_MASK | DMP_DEV_PRASE_FINISH;
    return true;
}


static LIST_HEAD(devlisthead);
static LIST_HEAD(devlistheadfree);
static void  initDevList() {
    static DMP_DEV devpool[40];
    memset(devpool, 0, sizeof(devpool));
    for (int i = 0; i < lenthof(devpool); i++) {
        list_add_tail(&devpool[i].list, &devlistheadfree);
    }
}

void praseDevList(CAN_DMP *frame, struct list_head *devlist) {
    char *buf;
    struct list_head *literal;
    DMP_DEV devxx;
    unsigned int packnum = (unsigned char)((frame->data[0] & 0xf000) >> 12);
    if (packnum * 6 >= sizeof(devxx.desc)) {
        return;
    }
    unsigned int ipack = (unsigned char)((frame->data[0] & 0xf00) >> 8);
    if (ipack >= packnum) {
        return;
    }
    buf = (char *)(frame->data) + 2;
    list_for_each(literal, devlist) {
        DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
        if (dev->uid == frame->uid) {
            strncpy(&(dev->desc[ipack * 6]), buf, 6);
            dev->ipacket++;
            if (dev->ipacket == packnum) {
                bool r = praseDev(dev->desc, dev);
                dev->stat = r ? DMP_DEV_PRASE_FINISH : DMP_DEV_PRASE_ERROR;
            }
            return;
        }
    }
    if (literal == devlist) {
        DMP_DEV *dev = list_first_entry(&devlistheadfree, DMP_DEV, list);
        list_del(&dev->list);
        dev->uid = frame->uid;
        dev->ipacket = 1;
        strncpy(&(dev->desc[0]), buf, 6);
        list_add_tail(&dev->list, devlist);
    }
}

static void dmpDevDataClear(DMP_DEV *dev) {
    struct list_head list;
    list.next = dev->list.next;
    list.prev = dev->list.prev;
    memset(dev, 0, sizeof(DMP_DEV));
    dev->list.next = list.next;
    dev->list.prev = list.prev;
}


void dmpCategDevice(DMP_SYSTEM *sys, struct list_head *devlist) {
    struct list_head *literal,*literal1,*n,*n1;
    for (int i = 0; i < 4; i++) {
        list_for_each_safe(literal, n, &sys->dev[i].regester) {
            DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
            list_for_each_safe(literal1, n1, devlist) {
                DMP_DEV *devs = list_entry(literal1, DMP_DEV, list);
                if ((devs->uid == dev->uid) && (devs->hdtype == dmpindex2devtype(i))) {
                    list_del(&devs->list);
                    list_replace(&dev->list, &devs->list);
                    devs->stat |= DMP_DEV_DEV_ONLINE;
                    devs->workid |= dev->workid;
                    dmpDevDataClear(dev);
                    list_add(&dev->list, &devlistheadfree);
                    break;
                }
            }
            if (literal1 == devlist) {
                dev->stat &= ~DMP_DEV_DEV_ONLINE;
            }
        }
    }
    list_for_each_safe(literal, n, devlist) {
        DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
        unsigned int index = dmpdevtype2index(dev->hdtype);
        if (index != -1UL) {
            if ((dev->stat == DMP_DEV_PRASE_FINISH) && (dev->inboot == 0)) {
                list_for_each_safe(literal1, n1, &sys->dev[index].newadd) {
                    DMP_DEV *devn = list_entry(literal1, DMP_DEV, list);
                    if (devn->uid == dev->uid) {
                        break;
                    }
                }
                if (literal1 == &sys->dev[index].newadd) {
                    list_move_tail(literal, &sys->dev[index].newadd);
                }
            } else {
                list_for_each_safe(literal1, n1, &sys->dev[index].unknow) {
                    DMP_DEV *devn = list_entry(literal1, DMP_DEV, list);
                    if (devn->uid == dev->uid) {
                        break;
                    }
                }
                if (literal1 == &sys->dev[index].unknow) {
                    list_move_tail(literal, &sys->dev[index].unknow);
                }
            }

        } else {
            list_for_each_safe(literal1, n1, &sys->unknow) {
                DMP_DEV *devn = list_entry(literal1, DMP_DEV, list);
                if (devn->uid == dev->uid) {
                    break;
                }
            }
            if (literal1 == &sys->unknow) {
                list_move_tail(literal, &sys->unknow);
            }
        }
    }
}


static  unsigned int listCnt(struct list_head *head) {
    struct list_head *literal;
    unsigned int i = 0;
    list_for_each(literal, head) {
        i++;
    }
    return i;
}


void dmpSysDevCnt(unsigned int devtypeIndex, unsigned int *regedCnt, unsigned int *offlineCnt, unsigned int *newCnt, unsigned int *unknowCnt) {
    if (devtypeIndex == -1UL) return;
    *newCnt = listCnt(&dmpSys.dev[devtypeIndex].newadd);
    *unknowCnt = listCnt(&dmpSys.dev[devtypeIndex].unknow);
    unsigned int off = 0, reg = 0;
    struct list_head *literal;
    list_for_each(literal, &dmpSys.dev[devtypeIndex].regester) {
        DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
        if (!(dev->stat & DMP_DEV_DEV_ONLINE)) {
            off++;
        }
        reg++;
    }
    *offlineCnt = off;
    *regedCnt = reg;
}


unsigned int  dmpSysWillRegCnt(unsigned int typeindex) {
    unsigned int regCnt = listCnt(&dmpSys.dev[typeindex].regester);
    unsigned int num = dmpSys.num[typeindex];
    return num - regCnt;
}



bool dmpWillRegAuto(unsigned int devTypeIndex) {
    unsigned int reg, offline, newadd, unknow;
    dmpSysDevCnt(devTypeIndex, &reg, &offline, &newadd, &unknow);
    if ((offline == 1) && (newadd == 1)) {
        return true;
    }
    return false;
}

bool dmpHaveNewAdd(unsigned int devTypeIndex) {
    unsigned int reg, offline, newadd, unknow;
    if (devTypeIndex==-1UL) {
        return false;
    }
    dmpSysDevCnt(devTypeIndex, &reg, &offline, &newadd, &unknow);
    if (newadd > 0) {
        return true;
    }
    return false;
}


bool dmpHaveOffline(unsigned int devTypeIndex) {
    unsigned int reg, offline, newadd, unknow;
    bool fg_offline = false;
    if (devTypeIndex != -1UL) {
        dmpSysDevCnt(devTypeIndex, &reg, &offline, &newadd, &unknow);
        if (offline > 0) {
            fg_offline = true;
        }
    } else {
        for (int i = 0; i < 4; i++) {
            dmpSysDevCnt(devTypeIndex, &reg, &offline, &newadd, &unknow);
            if (offline > 0) {
                fg_offline = true;
            }
        }

    }
    return fg_offline;
}



bool dmpWillUnReg(unsigned int devTypeIndex) {
    unsigned int reg, offline, newadd, unknow;
    if (devTypeIndex == -1UL) {
        return false;
    }
    dmpSysDevCnt(devTypeIndex, &reg, &offline, &newadd, &unknow);
    if (reg > 0) {
        return true;
    }
    return false;
}

bool dmpSysSave() {
    unsigned char buf[512];
    memset(buf, 0, sizeof buf);
    unsigned int *num = (unsigned int *)(buf + 4);
    unsigned int *magic = (unsigned int *)buf;
    DMP_UID_ID *uid_id = (DMP_UID_ID *)(buf + 8);
    struct list_head *literal;
    for (int i = 0; i < 4; i++) {
        list_for_each(literal, &dmpSys.dev[i].regester) {
            DMP_DEV *dev = list_entry(literal, DMP_DEV, list); ;
            uid_id[*num].uid = dev->uid;
            uid_id[*num].id = dev->workid;
            uid_id[*num].hdtype = dev->hdtype;
            (*num)++;
        }
    }
    *magic = 0x5555aaaa;
    return MMCSDP_Write(mmcsdctr, buf, DMPDEV_INAND_SECTOR, 1);
}



void dmpSysStore() {
    unsigned char buf[512];
    MMCSDP_Read(mmcsdctr, buf, DMPDEV_INAND_SECTOR, 1);
    unsigned int *magic = (unsigned int *)buf;
    unsigned int *num = (unsigned int *)(buf + 4);
    DMP_UID_ID *uid_id = (DMP_UID_ID *)(buf + 8);
    if (*magic != 0x5555aaaa) return;
    for (int i = 0; i < *num; i++) {
        unsigned int index = dmpdevtype2index(uid_id[i].hdtype);
        DMP_DEV *dev = list_entry(devlistheadfree.next, DMP_DEV, list);
        list_move_tail(devlistheadfree.next, &dmpSys.dev[index].regester);
        dev->uid = uid_id[index].uid;
        dev->workid = uid_id[index].id;
        dev->hdtype = uid_id[index].hdtype;
        dev->stat &= ~DMP_DEV_DEV_ONLINE;
    }
}



static DMP_DEV* dmpsys_find_offline(unsigned int devType) {
    struct list_head *literal;
    DMP_DEV *dev;
    list_for_each(literal, &devtype2devgroup(devType)->regester) {
        dev = list_entry(literal, DMP_DEV, list);
        if (!(dev->stat & DMP_DEV_DEV_ONLINE)) {
            break;
        }
    }
    return dev;
}


bool dmpAutoPreRegester(unsigned int devTypeIndex, unsigned int *id, DMP_DEV *newadddev) {
    unsigned int devType = dmpindex2devtype(devTypeIndex);
    if (!dmpWillRegAuto(devTypeIndex)) return false;
    DMP_DEV * devr,*devn;
    devr = dmpsys_find_offline(devType);
    devn = list_first_entry(&dmpSys.dev[devTypeIndex].newadd, DMP_DEV, list);
    newadddev = devn;
    bool r = dmpCanPreSetId(devn->uid, true, 10);
    if (r == false) {
        return r;
    }
    *id = devr->workid;
    return true;
}


bool dmpAutoRegester(unsigned int devTypeIndex) {
    unsigned int devType = dmpindex2devtype(devTypeIndex);
    if (!dmpWillRegAuto(devTypeIndex)) return false;
    DMP_DEV * devr,*devn;
    devr = dmpsys_find_offline(devType);
    devn = list_first_entry(&dmpSys.dev[devTypeIndex].newadd, DMP_DEV, list);
    unsigned int id = devr->workid;
    bool r;
    r = dmpCanSetId(devn->uid, id, 50);
    if (r == false) {
        return r;
    }
    devn->stat |= DMP_DEV_DEV_ONLINEMASK;
    list_move_tail(&devn->list, &dmpSys.dev[devTypeIndex].regester);
    dmpDevDataClear(devr);
    list_move(&devr->list, &devlistheadfree);
    return true;
}


DMP_DEV* dmpPreRegester1(unsigned int devTypeIndex, bool val) {
    if (!dmpHaveNewAdd(devTypeIndex)) return NULL;
    DMP_DEV *dev = list_first_entry(&dmpSys.dev[devTypeIndex].newadd, DMP_DEV, list);
    if (!dmpCanPreSetId(dev->uid, val, 20)) return NULL;		//10
    return dev;
}



bool dmpPreRegester2(DMP_DEV *dev, bool val) {
    if (!dmpCanPreSetId(dev->uid, val, 20)) return false;		//10 
    return true;
}


bool dmpRegester(DMP_DEV *dev, unsigned int id) {
    unsigned int uid = dev->uid;
    if (!dmpCanSetId(uid, id, 50)) return false;				//原来为30  出现未等到回码现象后修改为50 
    dev->workid = id;
    list_move_tail(&dev->list, &devtype2devgroup(dev->hdtype)->regester);
    dev->stat |= DMP_DEV_DEV_ONLINE;
    return true;
}




bool dmpUnRegester1(unsigned int devTypeIndex, unsigned int workId) {
    struct list_head *literal;
    DMP_DEV *dev;
    list_for_each(literal, &dmpSys.dev[devTypeIndex].regester) {
        dev = list_entry(literal, DMP_DEV, list);
        if (dev->workid == workId) {
            break;
        }
    }
    if (literal == &dmpSys.dev[devTypeIndex].regester) {
        return false;
    }
    if (dev->stat & DMP_DEV_DEV_ONLINE) {
        if (!dmpCanPreSetId(dev->uid, 1, 10)) return false;
        if (!dmpCanSetId(dev->uid, 0, 50)) return false;
        list_move_tail(&dev->list, &devtype2devgroup(dev->hdtype)->newadd);
    } else {
        dmpDevDataClear(dev);
        list_move_tail(&dev->list, &devlistheadfree);
    }
    return true;
}

bool  dmpUnregester2(unsigned int devTypeIndex) {
    return dmpUnRegester1(devTypeIndex, 0);
}

void  dmpUnregesterOffline(unsigned int devTypeIndex) {
    struct list_head *literal,*n;
    list_for_each_safe(literal, n, &dmpSys.dev[devTypeIndex].regester) {
        DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
        dmpDevDataClear(dev);
        list_move(literal, &devlistheadfree);
    }
}


void dmpInit() {
    //init dmpSys dmpSysSave
    for (int i = 0; i < 4; i++) {
        INIT_LIST_HEAD(&dmpSys.dev[i].regester);
        INIT_LIST_HEAD(&dmpSys.dev[i].newadd);
        INIT_LIST_HEAD(&dmpSys.dev[i].unknow);
    }
    INIT_LIST_HEAD(&dmpSys.unknow);
    initDevList();
    dmpSysStore();
}

bool dmpCheckDev() {
    //broadcast read device version
    struct list_head *literal,*n;
    list_for_each_safe(literal, n, &devlisthead) {
        DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
        dmpDevDataClear(dev);
        list_move(&dev->list, &devlistheadfree);
    }
    for (int i = 0; i < 4; i++) {
        list_for_each_safe(literal, n, &dmpSys.dev[i].newadd) {
            DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
            dmpDevDataClear(dev);
            list_move(&dev->list, &devlistheadfree);
        }
        list_for_each_safe(literal, n, &dmpSys.dev[i].unknow) {
            DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
            dmpDevDataClear(dev);
            list_move(&dev->list, &devlistheadfree);
        }
    }
    dmpCanRdDevVer(0xffffffff);
    delay(100);
    dmpCategDevice(&dmpSys, &devlisthead);
    return true;
}



void dmpCanRdDevVer(unsigned int devUid) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid =  devUid;
    frame.dlc =  1;
    frame.data[0] = DMP_FUNCODE_READVER;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}


bool dmpCanJumpToBoot(unsigned int devUid, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.dlc = 1;
    frame.data[0] = DMP_FUNCODE_JUMPBOOT;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&jumptobootflag);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&jumptobootflag)) {
            return true;
        }
    }
    return false;
}

bool dmpCanPreSetId(unsigned int devUid, bool val, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 2;
    frame.data[0] = (unsigned char)DMP_FUNCODE_PRESETID | (!!val) << 8;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&presetidflag);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&presetidflag)) {
            return true;
        }
    }
    return false;
}


bool dmpCanReadId(unsigned int devUid, unsigned char *id, unsigned int timeoutms) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 1;
    frame.data[0] = (unsigned char)DMP_FUNCODE_READID;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&readidflag);
    withintimedo(tmark, timeoutms) {
        if (atomicTestClear(&readidflag)) {
            *id = workid;
            return true;
        }
    }
    return false;
}




bool dmpCanSetId(unsigned int devUid, unsigned char id, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 2;
    frame.data[0] = (unsigned char)DMP_FUNCODE_SETID |
                    (unsigned char)id << 8;
    atomicClear(&setidflag);
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&setidflag)) {
            return true;
        }
    }
    return false;
}


bool dmpCanRcv(CAN_DMP *frame) {
    if (!isDmpFrameRcv(frame)) return false;
    unsigned char funcode = (unsigned char)(frame->data[0] & 0xff);
    switch (funcode) {
    case  DMP_FUNCODE_READVER:
        praseDevList(frame, &devlisthead);
        break;
    case  DMP_FUNCODE_SETID:
        atomicSet(&setidflag);
        break;
    case  DMP_FUNCODE_READID:
        workid = (frame->data[0] & 0xff00) >> 8;
        atomicSet(&readidflag);
        break;
    case DMP_FUNCODE_JUMPBOOT:
        atomicSet(&jumptobootflag);
        break;
    case DMP_FUNCODE_PRESETID:
        atomicSet(&presetidflag);
        break;
    default:
        break;
    }
    return true;
}
