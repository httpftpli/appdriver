
#include "pf_can.h"
#include "can_dmp.h"
#include "can_wp.h"
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
#include "algorithm.h"
#include "debug.h"


#define DMPDEV_INAND_SECTOR  USER_SECTOR+10


#define DMP_FUNCODE_READVER        0x01
#define DMP_FUNCODE_READID         	0x02
#define DMP_FUNCODE_PRESETID        0xfa
#define DMP_FUNCODE_SETID           0xfd
//#define DMP_FUNCODE_BURNAPP         0xf7


#define DMP_FUNCODE_JUMPAPP        	0xfc
#define DMP_FUNCODE_JUMPBOOT        0xfb
#define DMP_FUNCODE_ERASEAPP        0xf8
#define DMP_FUNCODE_ProgramDate     0xf7
#define DMP_FUNCODE_ProgramEnd      0xf6    //编程结束



#define DEFINE_CAN_DMP_FRAME(FRAME)  CAN_DMP FRAME = {.flag = 0,.dir=0,.candir=0,.xtd=1,.dlc=1}


extern mmcsdCtrlInfo mmcsdctr[2];
DMP_SYSTEM dmpSys = { .num = { 1, 2, 8, 1, 1 },};

void (*reghooks[DMP_DEV_HDTYPE_NUM])(DMP_DEV *dev);



static atomic setidflag,readidflag,jumptobootflag,presetidflag,jumptoappflag,
eraseappflag,programdataflag,programendflag,heartbeatflag,heartbeatenflag;
unsigned short workid;
unsigned int heartbeattimespan;
int CanDmp_Return = 0;
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
    case 'S' | 'e' << 8 | 'p' << 16:
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


//static bool praseDev(const char *desc, DMP_DEV *dev) {
bool praseDev(const char *desc, DMP_DEV *dev) {
    char *ver, *date, *p;
    char buf[50];
    strncpy(buf, desc, 49);
    buf[49] = 0;
    ver = strtok(buf, "&");
    date = strtok(NULL, "&");
    if ((ver == NULL) || (date == NULL)) {
        return false;
    }
    //prase ver
    p = strtok(ver, ".");
    if ((p == NULL) || (strlen(p) != 1)) return false;
    dev->inboot = (*p == 'A') ? 0 : 1;
    p = strtok(NULL, ".");
    if ((p == NULL) || (strlen(p) != 4)) return false;
    dev->hdtype = *((unsigned int *)p) & 0x00ffffff;
    dev->hdver = *(unsigned char *)(p + 3);
    p = strtok(NULL, ".");
    if ((p == NULL) || (strlen(p) > 9)) return false;
    //char ptemp[6] = { 0, 0, 0, 0, 0, 0 };
    strncpy((char *)dev->customcode, p, 9);
    //dev->customcode = *((unsigned int *)ptemp);
    p = strtok(NULL, ".");
    if ((p == NULL) || (strlen(p) != 2)) return false;
    dev->rombigver = (unsigned char)strtol(p, NULL, 10);
    p = strtok(NULL, ".");
    if ((p == NULL) || (strlen(p) != 2)) return false;
    dev->rommidver = (unsigned char)strtol(p, NULL, 10);
    p = strtok(NULL, ".");
    if ((p == NULL) || (strlen(p) != 2)) return false;
    dev->romlitver = (unsigned char)strtol(p, NULL, 10);

    //prase time
    struct tm tm_time;
    if (!strptime(date, &tm_time)) {
        return false;
    }
    dev->romtime = mktime(&tm_time);
    dev->stat = dev->stat & DMP_DEV_PARSE_MASK | DMP_DEV_PRASE_FINISH;
    return true;
}


static LIST_HEAD(devlisthead);
static LIST_HEAD(devlistheadfree);
static LIST_HEAD(heartbeatlist);
static void initDevList() {
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
    list = dev->list;
    memset(dev, 0, sizeof(DMP_DEV));
    dev->list = list;
}


static bool dmpdefaultheartbeart(DMP_DEV *dev);

void dmpCategDevice(DMP_SYSTEM *sys, struct list_head *devlist) {
    struct list_head *literal,*literal1,*n,*n1;
    //compare to regester list
    for (int i = 0; i < DMP_DEV_HDTYPE_NUM; i++) {
        list_for_each_safe(literal, n, &sys->dev[i].regester) {
            DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
            list_for_each_safe(literal1, n1, devlist) {
                DMP_DEV *devs = list_entry(literal1, DMP_DEV, list);
                if ((devs->stat == DMP_DEV_PRASE_FINISH) && (devs->uid == dev->uid) && (devs->hdtype == dmpindex2devtype(i))) {
                    if (isDevOnline(dev)) { //if old dev online ,don't replace
                        dmpDevDataClear(devs);
                        list_move_tail(&devs->list, &devlistheadfree);
                    } else {
                        if (devs->inboot == 0 & dmpCanSetId(devs->uid, dev->workid, 100)) {
                            devs->stat |= DMP_DEV_DEV_ONLINE;
                            devs->workid = dev->workid;
                            //call regester hooks
                            int typeindex = dmpdevtype2index(devs->hdtype);
                            ASSERT(typeindex < DMP_DEV_HDTYPE_NUM);
                            void (*reghook)(DMP_DEV *dev);
                            reghook = reghooks[typeindex];
                            if (reghook != NULL) {
                                reghook(devs);
                            }
                        } else {
                            dev->stat &= ~DMP_DEV_DEV_ONLINE;
                        }
                        list_del(&devs->list);
                        list_replace(&dev->list, &devs->list);
                        dmpDevDataClear(dev);
                        list_add(&dev->list, &devlistheadfree);
                    }
                    break;
                }
            }
            if (literal1 == devlist) {
                dev->stat &= ~DMP_DEV_DEV_ONLINE;
            }
        }
    }
    //compare to newadd list and unknow list
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

    //enable heartbeart ,get heartbeart time param
    /*for (int i = 0; i < DMP_DEV_HDTYPE_NUM; i++) {
        list_for_each(literal, &sys->dev[i].regester) {
            DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
            int32 timespan = 0;
            timespan = wpCanHeartbeatEn(dev->workid,1, 50);
            if (timespan != -1) {
                dmpDevSetHeartBeat(dev, dmpdefaultheartbeart, timespan);
            }
        }
    }*/
}


static unsigned int listCnt(struct list_head *head) {
    struct list_head *literal;
    unsigned int i = 0;
    list_for_each(literal, head) {
        i++;
    }
    return i;
}

static int candmpsort(const struct list_head *a, const struct list_head *b) {
    DMP_DEV *deva = list_entry(a, DMP_DEV, list);
    DMP_DEV *devb = list_entry(b, DMP_DEV, list);
    if (deva->workid > devb->workid) {
        return 1;
    } else if (deva->workid == devb->workid) {
        return 0;
    }
    return -1;
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


unsigned int dmpSysWillRegCnt(unsigned int typeindex, uint32 *flag) {
    unsigned int regCnt = listCnt(&dmpSys.dev[typeindex].regester);
    unsigned int num = dmpSys.num[typeindex];
    if (flag != NULL) {
        *flag = (1 << num) - 1;
        struct list_head *literal;
        DMP_DEV *dev;
        list_for_each(literal, &dmpSys.dev[typeindex].regester) {
            dev = list_entry(literal, DMP_DEV, list);
            *flag &= ~(1 << (CAN_WP_GET_ID(dev->workid) - 1));
        }
    }
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
    if (devTypeIndex == -1UL) {
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
        for (int i = 0; i < DMP_DEV_HDTYPE_NUM; i++) {
            dmpSysDevCnt(i, &reg, &offline, &newadd, &unknow);
            if (offline > 0) {
                fg_offline = true;
                break;
            }
        }

    }
    return fg_offline;
}

uint32 dmpsysListOffline(unsigned int devTypeIndex, DMP_DEV **devbuf, uint32 num) {
    struct list_head *literal;
    DMP_DEV *dev;
    int i = 0;
    list_for_each(literal, &dmpSys.dev[devTypeIndex].regester) {
        dev = list_entry(literal, DMP_DEV, list);
        if (!(isDevOnline(dev))) {
            if (i < num) {
                devbuf[i] = dev;
            }
            i++;
        }
    }
    return i;
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
    for (int i = 0; i < DMP_DEV_HDTYPE_NUM; i++) {
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
        if (index == 0xffffffff) {
            //TODO GAUDING
            continue;
        }
        DMP_DEV *dev = list_first_entry(&devlistheadfree, DMP_DEV, list);
        list_del(&dev->list);
        list_sort_insert(&dev->list, &dmpSys.dev[index].regester, candmpsort);
        //list_move_tail(&dev->list, &dmpSys.dev[index].regester);
        dev->uid = uid_id[i].uid;
        dev->workid = uid_id[i].id;
        dev->hdtype = uid_id[i].hdtype;
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


bool dmpAutoPreRegester(unsigned int devTypeIndex, unsigned int *id, DMP_DEV **newadddev) {
    unsigned int devType = dmpindex2devtype(devTypeIndex);
    if (!dmpWillRegAuto(devTypeIndex)) return false;
    DMP_DEV * devr,*devn;
    devr = dmpsys_find_offline(devType);
    devn = list_first_entry(&dmpSys.dev[devTypeIndex].newadd, DMP_DEV, list);
    *newadddev = devn;
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
    r = dmpRegester(devn, id);
    if (r == false) {
        return r;
    }
    dmpDevDataClear(devr);
    list_move(&devr->list, &devlistheadfree);
    return true;
}


DMP_DEV* dmpPreRegester1(unsigned int devTypeIndex, bool val) {
    if (!dmpHaveNewAdd(devTypeIndex)) return NULL;
    DMP_DEV *dev = list_first_entry(&dmpSys.dev[devTypeIndex].newadd, DMP_DEV, list);
    if (!dmpCanPreSetId(dev->uid, val, 20)) return NULL;        //10
    return dev;
}



bool dmpPreRegester2(DMP_DEV *dev, bool val) {
    if (!dmpCanPreSetId(dev->uid, val, 20)) return false;       //10
    return true;
}




bool dmpRegester(DMP_DEV *dev, unsigned int id) {
    unsigned int uid = dev->uid;
    if (!dmpCanSetId(uid, id, 50)) return false;                //原来为30  出现未等到回码现象后修改为50
    dev->workid = id;
    list_del(&dev->list);
    list_sort_insert(&dev->list, &devtype2devgroup(dev->hdtype)->regester, candmpsort);
    //list_move_tail(&dev->list, &devtype2devgroup(dev->hdtype)->regester);
    dev->stat |= DMP_DEV_DEV_ONLINE;
    //call regester hook;
    int typeindex = dmpdevtype2index(dev->hdtype);
    ASSERT(typeindex < DMP_DEV_HDTYPE_NUM);
    void (*reghook)(DMP_DEV *dev);
    reghook = reghooks[typeindex];
    if (reghook != NULL) {
        reghook(dev);
    }
    //set heartbeat
    /*int32 timespan = wpCanHeartbeatEn(dev->workid,1, 50);
    if (timespan > 0) {
        dev->timespan = timespan;
        dev->heartBeat = dmpdefaultheartbeart;
    } else {
        dev->timespan = 0;
        dev->heartBeat = NULL;
    }*/
    return true;
}


bool dmpDevRecoverId(unsigned int uid) {
    struct list_head *p;
    DMP_DEV *d = NULL;
    unsigned int typeindex;
    for (int i=0; i < DMP_DEV_HDTYPE_NUM; i++){
        list_for_each(p,&dmpSys.dev[i].regester){
            DMP_DEV *dev = list_entry(p,DMP_DEV,list);
            if (dev->uid == uid) {
                d = dev;
                typeindex = i;
                break;
            }
        }
    }
    if (d==NULL || !isDevOnline(d) ) {
        return false;
    }
    unsigned int id = d->workid;
    if (!dmpCanSetId(uid, id, 50)) return false;                //原来为30  出现未等到回码现象后修改为50
    
    //call regester hook;
    ASSERT(typeindex < DMP_DEV_HDTYPE_NUM);
    void (*reghook)(DMP_DEV *);
    reghook = reghooks[typeindex];
    if (reghook != NULL) {
        reghook(d);
    }
    return true;
}



void dmpDevSetRegHook(unsigned int devTypeIndex, void (*hook)(DMP_DEV *)) {
    ASSERT(devTypeIndex < DMP_DEV_HDTYPE_NUM);
    reghooks[devTypeIndex] = hook;
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

bool dmpUnregester2(unsigned int devTypeIndex) {
    return dmpUnRegester1(devTypeIndex, 0);
}

void dmpUnregesterOffline(unsigned int devTypeIndex) {
    struct list_head *literal, *n;
    list_for_each_safe(literal, n, &dmpSys.dev[devTypeIndex].regester) {
        DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
        if (!isDevOnline(dev)) {
            dmpDevDataClear(dev);
            list_move(literal, &devlistheadfree);
        }
    }
}

static void dmpDevSetHeartBeat(DMP_DEV *dev, BOOL(*heartBeat)(DMP_DEV *dev), uint32 timespan) {
    dev->heartBeat = heartBeat;
    dev->timespan = timespan;
    dev->__timespan_ = TimerTickGet64();
}

int32 wpDevHeartbeatEn(DMP_DEV *dev, bool en, bool (*heartbeart)(DMP_DEV *dev), unsigned int timeoutms) {
    if (dev == NULL) {
        return -1;
    }
    unsigned int id = dev->workid;
    /*if (en==false&&dev->timespan>0&&dev->heartBeat!=NULL) {
        dev->heartBeat(dev);
    }*/
    int timespan;
    int r = timespan = wpCanHeartbeatEn(id, en, timeoutms);
    if (timespan == -1) {
        timespan = 0;
    }
    if (!en) {
        timespan = 0;
    }
    if (heartbeart == NULL) {
        heartbeart = dmpdefaultheartbeart;
    }
    dmpDevSetHeartBeat(dev, heartbeart, timespan);
    return en ? r : 0;
}



void dmpInit() {
    //init dmpSys dmpSysSave
    for (int i = 0; i < DMP_DEV_HDTYPE_NUM; i++) {
        INIT_LIST_HEAD(&dmpSys.dev[i].regester);
        INIT_LIST_HEAD(&dmpSys.dev[i].newadd);
        INIT_LIST_HEAD(&dmpSys.dev[i].unknow);
    }
    INIT_LIST_HEAD(&dmpSys.unknow);
    INIT_LIST_HEAD(&dmpSys.heartbeartreturnlist);
    initDevList();
    dmpSysStore();
}


#define HEARTBEAT_RETURN_TIMEOUT   10 //MS
static uint16 heartbeart_id,heartbeart_en_id;
static bool dmpdefaultheartbeart(DMP_DEV *dev) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_HEARDBEAT;
    heartbeart_id = frame.desid = dev->workid;
    frame.dlc = 0;
    atomicClear(&heartbeatflag);
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    withintimedo(HEARTBEAT_RETURN_TIMEOUT) {
        if (atomicTestClear(&heartbeatflag)) {
            return true;
        }
    }
    return false;
}





DMP_DEV* dmpSysHeartbeat() {
    NOT_IN_IRQ();
    struct list_head *p;
    DMP_DEV *dev;
    for (int i = 0; i < DMP_DEV_HDTYPE_NUM; i++) {
        list_for_each(p, &dmpSys.dev[i].regester) {
            dev = list_entry(p, DMP_DEV, list);
            if (!isDevOnline(dev) || dev->timespan == 0) continue;
            unsigned long long timertick = TimerTickGet64();
            if (timertick >= dev->__timespan_) {
                dev->__timespan_ = timertick + dev->timespan;
                if (dev->heartBeat) {
                    if (dev->heartBeat(dev)) {
                        return NULL;
                    } else {
                        return dev;
                    }
                }
            }
        }
    }
    return NULL;
}



bool dmpDevHeartbeatEx(unsigned short workId) {
    NOT_IN_IRQ();
    unsigned int type = CAN_WP_GET_TYPE(workId);
    struct list_head *p;
    list_for_each(p,&dmpSys.dev[type-1].regester){
        DMP_DEV *dev = list_entry(p,DMP_DEV,list);
        if (dev->workid==workId && dev->heartBeat) {
            return dev->heartBeat(dev);
        }
    }
    return false;
}


bool dmpCheckDev() {
    //broadcast read device version
    struct list_head *literal,*n;
    list_for_each_safe(literal, n, &devlisthead) {
        DMP_DEV *dev = list_entry(literal, DMP_DEV, list);
        dmpDevDataClear(dev);
        list_move(&dev->list, &devlistheadfree);
    }
    for (int i = 0; i < DMP_DEV_HDTYPE_NUM; i++) {
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


void dmpIapHelper(DMP_IAP_HELPER *helper, uint32 num) {
    struct list_head *p,*n;
    DMP_DEV * dev,*helpdev;
    list_for_each_safe(p, n, &devlisthead) {
        dev = list_entry(p, DMP_DEV, list);
        dmpDevDataClear(dev);
        list_move(p, &devlistheadfree);
    }
    dmpCanRdDevVer(0xffffffff);
    delay(100);
    uint32 typeindex;
    for (int i = 0; i < num; i++) {
        helper[i].num = 0;
    }
    list_for_each(p, &devlisthead) {
        dev = list_entry(p, DMP_DEV, list);
        typeindex = dmpdevtype2index(dev->hdtype);
        if ((typeindex != 0xffffffff) && (typeindex < num)) {
            helpdev = &helper[typeindex].devs[helper[typeindex].num++];
            memcpy(helpdev, dev, sizeof*dev);
        }
    }
    list_for_each_safe(p, n, &devlisthead) {
        dev = list_entry(p, DMP_DEV, list);
        dmpDevDataClear(dev);
        list_move(p, &devlistheadfree);
    }
    for (int i = 0; i < num; i++) {
        for (int j = 0; j < helper[i].num; j++) {
            dmpCanReadId(helper[i].devs[j].uid, &helper[i].devs[j].workid, 10);
        }
    }
}

void dmpCanRdDevVer(unsigned int devUid) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 1;
    frame.data[0] = DMP_FUNCODE_READVER;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

bool dmpCanPreSetId(unsigned int devUid, bool val, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 2;
    frame.data[0] = (unsigned char)DMP_FUNCODE_PRESETID | (!!val) << 8;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&presetidflag);
    withintimedo(timeout) {
        if (atomicTestClear(&presetidflag)) {
            return true;
        }
    }
    return false;
}


bool dmpCanReadId(unsigned int devUid, unsigned short *id, unsigned int timeoutms) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 1;
    frame.data[0] = (unsigned char)DMP_FUNCODE_READID;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&readidflag);
    withintimedo(timeoutms) {
        if (atomicTestClear(&readidflag)) {
            *id = workid;
            return true;
        }
    }
    return false;
}




bool dmpCanSetId(unsigned int devUid, unsigned short id, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 3;
    frame.data[0] = (unsigned char)DMP_FUNCODE_SETID | id << 8;
    atomicClear(&setidflag);
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    withintimedo(timeout) {
        if (atomicTestClear(&setidflag)) {
            return true;
        }
    }
    return false;
}


//#########################################################################
bool dmpCanJumpToBoot(unsigned int devUid, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 1;
    frame.data[0] = DMP_FUNCODE_JUMPBOOT;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&jumptobootflag);
    withintimedo(timeout) {
        if (atomicTestClear(&jumptobootflag)) {
            return true;
        }
    }
    return false;
}

bool dmpCanJumpToApp(unsigned int devUid, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 1;
    frame.data[0] = DMP_FUNCODE_JUMPAPP;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&jumptoappflag);
    withintimedo(timeout) {
        if (atomicTestClear(&jumptoappflag)) {
            return true;
        }
    }
    return false;
}
bool dmpCanBootEraseApp(unsigned int devUid, unsigned int max_baohao, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 4;
    //frame.data[0] = DMP_FUNCODE_ERASEAPP;
    frame.data[0] = (unsigned char)DMP_FUNCODE_ERASEAPP |
                    (unsigned int)(max_baohao & 0xffffff) << 8;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&eraseappflag);
    withintimedo(timeout) {
        if (atomicTestClear(&eraseappflag)) {
            return true;
        }
    }
    return false;
}
bool dmpCanProgramDate(unsigned int devUid, unsigned int baohao, unsigned char count, unsigned char *val, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 4 + count;
    frame.data[0] = (unsigned char)DMP_FUNCODE_ProgramDate |
                    (unsigned int)(baohao & 0xffffff) << 8;
    memcpy(&frame.data[1], val, count);
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&programdataflag);
    withintimedo(timeout) {
        if (atomicTestClear(&programdataflag)) {
            return true;
        }
    }
    return false;
}

bool dmpCanProgramEnd(unsigned int devUid, unsigned int timeout) {
    DEFINE_CAN_DMP_FRAME(frame);
    frame.uid = devUid;
    frame.dlc = 1;
    frame.data[0] = DMP_FUNCODE_ProgramEnd;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&programendflag);
    withintimedo(timeout) {
        if (atomicTestClear(&programendflag)) {
            return true;
        }
    }
    return false;
}



int32 wpCanHeartbeatEn(unsigned int id, bool en, unsigned int timeoutms) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.desid = id;
    frame.dlc = 1;
    frame.data[0] = !!en;
    frame.funcode = (unsigned char)CAN_WP_FUNCODE_HEARDBEATEN;
    heartbeart_en_id = id;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&heartbeatenflag);
    withintimedo(timeoutms) {
        if (atomicTestClear(&heartbeatenflag)) {
            return heartbeattimespan;
        }
    }
    return -1;
}


bool wpHeartBeartCanRcv(CAN_WP *frame) {
    switch (frame->funcode) {
    case CAN_WP_FUNCODE_HEARDBEAT:
        if (heartbeart_id == frame->srcid) {
            atomicSet(&heartbeatflag);
            return true;
        }
        break;
    case CAN_WP_FUNCODE_HEARDBEATEN:
        if (heartbeart_en_id == frame->srcid) {
            atomicSet(&heartbeatenflag);
            heartbeattimespan = (unsigned short)frame->data[0] & 0xffff;
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}



bool dmpCanRcv(CAN_DMP *frame) {
    if (!isDmpFrameRcv(frame)) return false;
    unsigned char funcode = (unsigned char)(frame->data[0] & 0xff);
    switch (funcode) {
    case DMP_FUNCODE_READVER:
        praseDevList(frame, &devlisthead);
        break;
    case DMP_FUNCODE_SETID:
        atomicSet(&setidflag);
        break;
    case DMP_FUNCODE_READID:
        workid = (frame->data[0] & 0xffff00) >> 8;
        atomicSet(&readidflag);
        break;
    case DMP_FUNCODE_PRESETID:
        atomicSet(&presetidflag);
        break;
    case DMP_FUNCODE_JUMPAPP:
        atomicSet(&jumptoappflag);
        break;
    case DMP_FUNCODE_JUMPBOOT:
        atomicSet(&jumptobootflag);
        break;
    case DMP_FUNCODE_ERASEAPP:
        atomicSet(&eraseappflag);
        CanDmp_Return = (frame->data[0] & 0xff00) >> 8;
        break;
    case DMP_FUNCODE_ProgramDate:
        atomicSet(&programdataflag);
        CanDmp_Return = ((frame->data[0] & 0xffffff00) >> 8) | ((frame->data[1] & 0xff) << 24);
        break;
    case DMP_FUNCODE_ProgramEnd:
        atomicSet(&programendflag);
        break;
    default:
        break;
    }
    return true;
}
