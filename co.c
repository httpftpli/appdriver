#include "type.h"
#include "list.h"
#include "ff.h"
#include <string.h>
#include "pf_platform_cfg.h"
#include "mmath.h"
#include "co.h"
#include "algorithm.h"
#include "debug.h"



static FUNC __func[1000];
static struct list_head funcfreelist;


static FENGMEN __fengmen[1000];
static struct list_head fengmenfreelist;

static S_CO_RUN_STEP __run_step[500];
static struct list_head runstepfreelist;

static WELT_PARAM __welt_param_pool[20];
static struct list_head welt_param_freelist;

static ACT_GROUP act360[360];

void coInit() {
    INIT_LIST_HEAD(&funcfreelist);
    INIT_LIST_HEAD(&fengmenfreelist);
    INIT_LIST_HEAD(&runstepfreelist);
    INIT_LIST_HEAD(&welt_param_freelist);
    for (int i = 0; i < lenthof(__func); i++) {
        list_add_tail(&__func[i].list, &funcfreelist);
    }
    for (int i = 0; i < lenthof(__fengmen); i++) {
        list_add_tail(&__fengmen[i].list, &fengmenfreelist);
    }
    for (int i = 0; i < lenthof(__run_step); i++) {
        list_add_tail(&__run_step[i].list, &runstepfreelist);
    }
    for (int i = 0; i < lenthof(__welt_param_pool); i++) {
        list_add_tail(&__welt_param_pool[i].list, &welt_param_freelist);
    }
}


static bool readSinkMotor(FIL *file, SINKERMOTOR_ZONE *zone, unsigned int *num) {
    *num = 0;
    while (1) {
        CO_SINKERMOTOR_ZONE co_zone;
        uint32 br;
        FRESULT r = f_read(file, &co_zone, sizeof co_zone,&br);
        if (r != FR_OK || br != sizeof co_zone) {
            //re = CO_FILE_READ_ERROR;
            return false;
        }
        if (co_zone.size != 0) {
            SINKERMOTOR_ZONE *zonetemp = zone + *num;

            memcpy(&zonetemp->descrpition, &co_zone.descrpition, sizeof zonetemp->descrpition);

            zonetemp->head.angle = co_zone.head.angle;
            zonetemp->head.beginStep = co_zone.head.beginStep;
            zonetemp->head.endStep = co_zone.head.endStep;
            zonetemp->head.groupNum = co_zone.head.groupNum;

            for (int i = 0; i < 8; i++) {
                zonetemp->param[i].qf_feed = co_zone.param[i].qf_feed[0];
                zonetemp->param[i].qi_feed = co_zone.param[i].qi_feed[0];
            }
            (*num)++;
        } else {
            f_lseek(file, f_tell(file) - sizeof co_zone + 4); //back filepoint
            break;
            //TODO: numof sizemotor zone protect
        }
    }
    return true;
}



static bool readSizemotor(FIL *file, SIZEMOTOR_ZONE *zone, unsigned int *num) {
    *num = 0;
    while (1) {
        CO_SIZEMOTOR_ZONE co_szone;
        uint32 br;
        FRESULT r = f_read(file, &co_szone, sizeof co_szone,&br);
        if (r != FR_OK || br != sizeof co_szone) {
            //re = CO_FILE_READ_ERROR;
            return false;
        }
        if (co_szone.size != 0) {
            SIZEMOTOR_ZONE *zone_temp = zone + *num;
            memcpy(&zone_temp->descrpition, &co_szone.descrpition, sizeof zone->descrpition);

            zone_temp->head.angle = co_szone.head.angle;
            zone_temp->head.beginStep = co_szone.head.beginStep;
            zone_temp->head.endStep = co_szone.head.endStep;
            zone_temp->head.groupNum = co_szone.head.groupNum;

            for (int i = 0; i < 8; i++) {
                zone_temp->param[i].start = co_szone.param[i].start[0];
                zone_temp->param[i].startWidth = co_szone.param[i].startWidth;
                zone_temp->param[i].startWidthDec = co_szone.param[i].startWidthDec;
                zone_temp->param[i].end = co_szone.param[i].end[0];
                zone_temp->param[i].endWidth = co_szone.param[i].endWidth;
                zone_temp->param[i].endWidthDec = co_szone.param[i].endWidthDec;
            }
            (*num)++;
        } else {
            f_lseek(file, f_tell(file) - sizeof co_szone + 4); //back filepoint
            break;
            //TODO: numof sizemotor zone protect
        }
    }
    return true;
}


static bool readSpeed(FIL *file, SPEED *speed, unsigned int *num) {
    *num = 0;
    while (1) {
        CO_SPEED co_speed;
        uint32 br;
        FRESULT r = f_read(file, &co_speed, sizeof co_speed,&br);
        if (r != FR_OK || br != sizeof co_speed) {
            //re = CO_FILE_READ_ERROR;
            return false;
        }
        if (co_speed.step != 0xffffffff) {
            SPEED *speedtemp = speed + *num;
            memcpy(speedtemp->ramp, co_speed.ramp, sizeof speedtemp->ramp);
            speedtemp->rpm = co_speed.rpm[0];
            speedtemp->step = co_speed.step;
            (*num)++;
        } else {
            f_lseek(file, f_tell(file) - sizeof co_speed + 4); //back filepoint
            break;
            //TODO: numof SPEED protect
        }
    }
    return true;
}


bool readFunc(FIL *file, struct list_head *funclist, unsigned int *step) {
    *step = 0;
    unsigned fileoffset = f_tell(file);
    unsigned int size, br;
    FRESULT r = f_read(file, &size, sizeof size,&br);
    if (r != FR_OK || br != sizeof size) {
        //re = CO_FILE_READ_ERROR;
        return false;
    }
    *step = size / 4;
    for (int i = 0; i < *step; i++) {
        unsigned int addr;
        //read addr
        f_lseek(file, fileoffset + i * 4);
        r = f_read(file, &addr, 4, &br);
        if (r != FR_OK || br != 4) {
            //re = CO_FILE_READ_ERROR;
            return false;
        }
        addr += fileoffset;
        f_lseek(file, addr);
        //read func;
        CO_FUNC cofunc;
        while (1) {
            FUNC *func = list_entry(funcfreelist.next, FUNC, list);
            r = f_read(file, &cofunc, 2, &br);
            if (cofunc.angular != 0x8000) {
                r = f_read(file, &cofunc.value, 4, &br);
                if (cofunc.funcode == 0x031f) {
                    r = f_read(file, &cofunc.add, 4, &br);
                } else if (cofunc.funcode == 0x030b) {
                    r = f_read(file, &cofunc.add, 2, &br);
                }
                func->angular = cofunc.angular;
                func->value = cofunc.value;
                func->funcode = cofunc.funcode;
                func->funcode = cofunc.funcode;
                *(uint32 *)func->add = *(uint32 *)cofunc.add;
                list_move_tail(&func->list, &funclist[i]);
            } else {
                break;
            }
        }
    }
    return true;
}


static bool readFengmen(FIL *file, struct list_head *fengmenlist, unsigned int *step) {
    *step = 0;
    unsigned fileoffset = f_tell(file);
    unsigned int size, br;
    FRESULT r = f_read(file, &size, sizeof size,&br);
    if (r != FR_OK || br != sizeof size) {
        //re = CO_FILE_READ_ERROR;
        return false;
    }
    *step = size / 4;
    for (int i = 0; i < *step; i++) {
        unsigned int addr;
        //read addr
        f_lseek(file, fileoffset + i * 4);
        r = f_read(file, &addr, 4, &br);
        if (r != FR_OK || br != 4) {
            //re = CO_FILE_READ_ERROR;
            return false;
        }
        addr += fileoffset;
        f_lseek(file, addr);
        //read func;
        FENGMEN *fengmen;
        while (1) {
            fengmen = list_entry(fengmenfreelist.next, FENGMEN, list);
            r = f_read(file, fengmen, 2, &br);
            if (fengmen->angular != 0x8000) {
                r = f_read(file, &fengmen->angular, sizeof(FENGMEN) - 2 - sizeof(struct list_head), &br);
                list_move_tail(&fengmen->list, &fengmenlist[i]);
            } else {
                break;
            }
        }
    }
    return true;
}


static bool readMotorHeader(FIL *file, MOTOR_HEADER_PARAM *motor, unsigned int num) {
    f_lseek(file, f_tell(file) + 4);
    uint32 br;
    FRESULT r;
    for (int i = 0; i < num; i++) {
        r = f_read(file, motor + i, sizeof(MOTOR_HEADER_PARAM), &br);
        if (r != FR_OK || br != sizeof(MOTOR_HEADER_PARAM)) {
            return false;
        }
    }
    return true;
}


static bool readWelt(FIL *file, struct list_head *weltListHead, uint32 num) {
    CO_WELT_PARAM co_welt;
    uint32 br;
    for (int i = 0; i < num; i++) {
        FRESULT r = f_read(file, &co_welt, sizeof co_welt ,&br);
        if (r != FR_OK || br != sizeof co_welt) {
            return false;
        }
        ASSERT(co_welt.weltflag == 0x2042 || co_welt.weltflag == 0x2062);
        WELT_PARAM *welt;
        if (co_welt.weltflag == 0x2042) { //welt in
            welt = list_entry(welt_param_freelist.next, WELT_PARAM, list);
            welt->weltinstep = co_welt.step;
            list_move_tail(welt_param_freelist.next, weltListHead);
        } else if (co_welt.weltflag == 0x2062) { //welt out
            welt->weltoutstep = co_welt.step - 1;
        }
    }
    //end flag 4byte should be 0
    uint32 temp;
    FRESULT r = f_read(file, &temp, 4, &br);
    if (r != FR_OK || br != 4 || temp != 0) {
        return false;
    }
    return true;
}

static bool readEconomizer(FIL *file, ECONOMIZER_PARAM *econo, uint32 num) {
    CO_ECONOMIZER_PARAM param;
    uint32 br;
    FRESULT r;
    for (int i = 0; i < num; i++) {
        r = f_read(file, &param, sizeof param ,&br);
        if (r != FR_OK || br != sizeof param) {
            return false;
        }
        econo[i].begin = param.begin;
        econo[i].end = param.end;
        for (int j = 0; j < 8; j++) {
            econo[i].economize[j] = param.economize[j][0];
        }
    }
    //read end 4 byte ,should be 0x00;
    uint32 numtemp;
    r = f_read(file, &numtemp, 4, &br);
    if (r != FR_OK || br != 4 || numtemp != 0) {
        return false;
    }
    return true;
}


static bool coCheck(S_CO *co) {
    if (co->numofspeed < 1 || co->numofspeed > co->numofstep) {
        return false;
    }
    return true;
}


bool coMd5(const TCHAR *path, void *md5, int md5len) {
    ASSERT(md5len >= 16);
    uint8 buf[1024];
    FIL file;
    if (f_open(&file, path, FA_READ) != FR_OK) {
        return false;
    }
    uint32 br;
    MD5_CTX context;
    MD5Init(&context);
    while (1) {
        if (f_read(&file, buf, sizeof buf,&br) != FR_OK) {
            f_close(&file);
            return false;
        }
        MD5Update(&context, buf, br);
        if (br != sizeof buf) {
            MD5Final(&context, (unsigned char *)md5);
            f_close(&file);
            return true;
        }
    }
    return false;
}


bool coParse(const TCHAR *path, S_CO *co, unsigned int *offset) {
    FIL file;
    unsigned int br;


    //do co struct init
    for (int i = 0; i < lenthof(co->func); i++) {
        INIT_LIST_HEAD(&co->func[i]);
    }
    for (int i = 0; i < lenthof(co->fengmen); i++) {
        INIT_LIST_HEAD(&co->fengmen[i]);
    }
    INIT_LIST_HEAD(&co->welt);

    //open co file
    FRESULT r = f_open(&file, path, FA_READ);
    if (r != FR_OK) {
        //re = CO_FILE_OPEN_ERROR;
        return false;
    }
    unsigned char buf[512];

    r = f_read(&file, buf, 512, &br);
    if (r != FR_OK || br != 512) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    unsigned char ver = buf[28];


    CO_PART1_ATTRIB *coattri1 = (CO_PART1_ATTRIB *)buf;
    r = f_read(&file, coattri1, sizeof*coattri1,&br);
    if (r != FR_OK || br != sizeof*coattri1) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }

    co->diameter = coattri1->diameter;
    co->niddle = coattri1->niddle;

    //justify address ;
    coattri1->sizemotorAddr += 512;
    coattri1->sinkermotor_feed1_3_Addr += 512;
    coattri1->stitch1Addr += 512;
    coattri1->stitch2Addr += 512;
    coattri1->stitch3Addr += 512;
    coattri1->stitch4Addr += 512;
    coattri1->sinkermotor_feed2_4_Addr += 512;
    coattri1->sinkerangular_Addr += 512;
    //read sizemoter;
    f_lseek(&file, coattri1->sizemotorAddr);
    readSizemotor(&file, co->sizemotor, &co->numofsizemotorzone);

    //read sinkermotor_feed1_3;
    f_lseek(&file, coattri1->sinkermotor_feed1_3_Addr);
    if (!readSinkMotor(&file, co->sinkmoterzone_1_3, &co->numofsinkmoterzone_1_3)) {
        goto ERROR;
    }
    //read sinkermotor_feed2_4;
    f_lseek(&file, coattri1->sinkermotor_feed2_4_Addr);
    if (!readSinkMotor(&file, co->sinkmoterzone_2_4, &co->numofsinkmoterzone_2_4)) {
        goto ERROR;
    }

    //read sinkerangular;
    f_lseek(&file, coattri1->sinkerangular_Addr);
    if (!readSinkMotor(&file, co->sinkangular, &co->numofsinkangular)) {
        goto ERROR;
    }


    //512 aligne
    uint32 coPart2Offset = BOUNDUP(512 + coattri1->co_size, 256);
    f_lseek(&file, coPart2Offset);
    //read part2 of CO ,including chain;

    CO_PART2_ATTRIB *coattri2 = (CO_PART2_ATTRIB *)buf;
    r = f_read(&file, coattri2, sizeof*coattri2,&br);
    if (r != FR_OK || br != sizeof*coattri2) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    coattri2->unkownaddr[11] += coPart2Offset;
    f_lseek(&file, coattri2->unkownaddr[11]);
    uint32 lastmotor;
    r = f_read(&file, &lastmotor, sizeof lastmotor,&br);
    if (r != FR_OK || br != 4) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (lastmotor != 0) {
        goto ERROR;
    }

    //read MOTOR_HEADER_GROUP
    if (ver == '6') {
        if (!readMotorHeader(&file, co->motor_header, 12)) {
            goto ERROR;
        }
    }

//read part 3 of co
    uint32 coPart3Offset = BOUNDUP(f_tell(&file), 256);
    f_lseek(&file, coPart3Offset);
    CO_PART3_ATTRIB *coattri3 = (CO_PART3_ATTRIB *)buf;
    r = f_read(&file, coattri3, sizeof*coattri3,&br);
    if (r != FR_OK || br != sizeof*coattri3) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }

    coattri3->speed_addr += coPart3Offset;
    coattri3->func_addr += coPart3Offset;
    co->file_speedOff = coattri3->func_addr;
    f_lseek(&file, coattri3->speed_addr);
    if (!readSpeed(&file, co->speed, &co->numofspeed)) {
        goto ERROR;
    }

//read funcode;
    f_lseek(&file, coattri3->func_addr);
    unsigned int step;
    readFunc(&file, co->func, &step);
    co->numofstep = step;

//read part 4 of co
    unsigned int coPart4Offset = BOUNDUP(f_tell(&file), 256);
    CO_PART4_ATTRIB *coattri4 = (CO_PART4_ATTRIB *)buf;
    f_lseek(&file, coPart4Offset);
    r = f_read(&file, coattri4, sizeof*coattri4,&br);
    if (r != FR_OK || br != sizeof*coattri4) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }

//read fengmen
    coattri4->fengmen2addr += coPart4Offset;
    coattri4->fengmen1addr += coPart4Offset;
    f_lseek(&file, coattri4->fengmen2addr);
    if (!readFengmen(&file, co->fengmen, &step) || step != co->numofstep) {
        goto ERROR;
    }
    f_lseek(&file, coattri4->fengmen1addr * 2 - coattri4->fengmen2addr);

    //skip 2 unknow block
    unsigned int unknowOffset = BOUNDUP(f_tell(&file), 256);
    f_lseek(&file, unknowOffset);

    uint32 unknow1;
    while (1) {
        r = f_read(&file, &unknow1, 4, &br);
        if (r != FR_OK || br != 4) {
            goto ERROR;
        }
        if (unknow1 != 0xff) {
            f_lseek(&file, f_tell(&file) + 256 - 4);
        } else {
            break;
        }
    }

    f_lseek(&file, f_tell(&file) - 4);

    //read co_part5
    CO_ATTRIB5 co_attib5;
    r = f_read(&file, &co_attib5, sizeof co_attib5 ,&br);
    if (r != FR_OK || br != sizeof co_attib5) {
        goto ERROR;
    }
    //read welt;
    if (!readWelt(&file, &co->welt,
                  (co_attib5.econoAddrOffset - co_attib5.weltAddrOffset) / sizeof(CO_WELT_PARAM))) {
        goto ERROR;
    }
    //read economizers
    co->numofeconomizer = (co_attib5.size - co_attib5.econoAddrOffset) / sizeof(CO_ECONOMIZER_PARAM);
    if (!readEconomizer(&file, co->econo, co->numofeconomizer)) {
        goto ERROR;
    }

    f_close(&file);
    if (!coCheck(co)) return false;
    return true;

    ERROR:
    *offset = (uint32)f_tell(&file);
    f_close(&file);
    return false;
}


void coRelease(S_CO *co) {
    struct list_head *p,*n;
    FUNC *func;
    for (int i = 0; i < lenthof(co->func); i++) {
        list_for_each_safe(p, n, &co->func[i]) {
            func = list_entry(p, FUNC, list);
            memset(func, 0, sizeof(FUNC) - sizeof(func->list)); //clear func but list member
            list_move(p, &funcfreelist);
        }
    }
    FENGMEN *fengmen;
    for (int i = 0; i < lenthof(co->fengmen); i++) {
        list_for_each_safe(p, n, &co->fengmen[i]) {
            fengmen = list_entry(p, FENGMEN, list);
            memset(fengmen, 0, sizeof(FENGMEN) - sizeof(fengmen->list)); //clear fenmen but list member
            list_move(p, &fengmenfreelist);
        }
    }
    //release welt
    WELT_PARAM *welt;
    list_for_each_safe(p, n, &co->welt) {
        welt = list_entry(p, WELT_PARAM, list);
        memset(welt, 0, sizeof(WELT_PARAM) - sizeof(welt->list)); //clear welt param but list member
        list_move(p, &welt_param_freelist);
    }

    //release CO_RUN
    if (co->run != NULL) {
        S_CO_RUN *run = co->run;
        struct list_head *p, *n;
        list_for_each_safe(p, n, &run->step) {
            S_CO_RUN_STEP *step = list_entry(p, S_CO_RUN_STEP, list);
            list_del(p);
            memset(step, 0, sizeof*step);
            list_add(p, &runstepfreelist);
        }
        memset(run, 0, sizeof*run);
    }
    memset(co, 0, sizeof*co);
}



static void cocreateindex_speed(S_CO_RUN *co_run, S_CO *co) {
    uint32 ispeed = 0;
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (co->speed[ispeed].step == i) {
            step->speed = &co->speed[ispeed++];
        } else {
            step->speed = NULL;
        }
    }
}


static void cocreateindex_func(S_CO_RUN *co_run, S_CO *co) {
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (list_empty(&co->func[i])) {
            step->func = NULL;
        } else {
            step->func = &co->func[i];
        }
    }
}



static void cocreateindex_econ(S_CO_RUN *co_run, S_CO *co) {
    uint32 iecon = 0;
    if (co->numofeconomizer == 0) {
        for (int i = 0; i < 8; i++) {
            co_run->numofline[i] = co->numofstep;
        }
        for (int i = 0; i < co->numofstep; i++) {
            co_run->stepptr[i]->econo = NULL;
            co_run->stepptr[i]->econoFlag = 0;
        }
        return;
    }
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (co->econo[iecon].end == i) { //tag economizer begin
            step->econoFlag |= ECONO_END;
            step->econo = &co->econo[iecon];
            //back to econo begin
            S_CO_RUN_STEP *beginstep = co_run->stepptr[co->econo[iecon].begin];
            beginstep->econoFlag |= ECONO_BEGIN;
            beginstep->econo = &co->econo[iecon];
            unsigned econodif = co->econo[iecon].end - co->econo[iecon].begin + 1;
            for (int j = 0; j < 8; j++) {
                co_run->numofline[j] += econodif * co->econo[iecon].economize[j];
                if (i != 0) {
                    co_run->stepptr[i]->ilinetag[j] = co_run->stepptr[i - 1]->ilinetag[j]
                                                      + econodif * co->econo[iecon].economize[j];
                }
            }
            iecon++;
        } else {
            step->econo = NULL;
            step->econoFlag = 0;
            for (int j = 0; j < 8; j++) {
                co_run->numofline[j]++;
                if (i != 0) {
                    co_run->stepptr[i]->ilinetag[j] = co_run->stepptr[i - 1]->ilinetag[j] + 1;
                }
            }
        }
    }
}


static void cocreateindex_sizemotor(S_CO_RUN *co_run, S_CO *co) {
    uint32 isizemotorzone = 0;
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (co->sizemotor[isizemotorzone].head.beginStep <= i && co->sizemotor[isizemotorzone].head.endStep >= i) {
            step->sizemotor = &co->sizemotor[isizemotorzone];
            if (co->sizemotor[i].head.endStep == i) {
                isizemotorzone++;
            }
        } else {
            step->sizemotor = NULL;
        }
    }
    for (int i = 0; i < co->numofsizemotorzone; i++) {
        SIZEMOTOR_ZONE *zone = &co->sizemotor[isizemotorzone];
        uint32 beginstep = zone->head.beginStep;
        uint32 endstep = zone->head.endStep;
        for (int j = 0; j < 8; j++) {
            uint32 linediff = co_run->stepptr[endstep]->ilinetag - co_run->stepptr[beginstep]->ilinetag;
            if (linediff != 0) {
                zone->param[j].acc = (zone->param[j].end - zone->param[j].start) * 1000 / linediff;
            } else {
                zone->param[j].acc = 0;
            }
        }
    }
}

static void cocreateindex_welt(S_CO_RUN *co_run, S_CO *co) {
    //clear all welt flag;
    for (int i = 0; i < co->numofstep; i++) {
        co_run->stepptr[i]->welt = false;
    }

    //set welt flag;
    struct list_head *p;

    list_for_each(p, &co->welt) {
        WELT_PARAM *param = list_entry(p, WELT_PARAM, list);
        for (int i = param->weltinstep; i <= param->weltoutstep; i++) {
            co_run->stepptr[i]->welt = true;
        }
    }
}



void coCreateIndex(S_CO_RUN *co_run, S_CO *co) {
    INIT_LIST_HEAD(&co_run->step);
    //create index and list
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = list_entry(runstepfreelist.next, S_CO_RUN_STEP, list);
        co_run->stepptr[i] = step;
        list_move_tail(&step->list, &co_run->step);
    }

    //speed;
    cocreateindex_speed(co_run, co);
    //welt;
    cocreateindex_welt(co_run, co);
    //economizer
    cocreateindex_econ(co_run, co);

    //sizemotor;
    cocreateindex_sizemotor(co_run, co);
    //func
    cocreateindex_func(co_run, co);


    co_run->numofstep = co->numofstep;
    co->run = co_run;
    co->run->istep = 0;
    co_run->speedAcc = 0;
    co_run->co = co;
    co_run->iecono = 0;
    co_run->econonum = 0;
    co_run->econostepfrom = 0;
    co_run->econostepto = 0;
    co_run->nextline = 0;
    co_run->prerpm = 0;
    co_run->rpm = 0;

    co_run->act = act360;
}


static void funcodeParse(struct list_head *func, ACT_GROUP *angleValve);
uint32 corunReadLine(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size) {
    uint32 stepindex = co_run->istep = co_run->nextstep;
    co_run->prerpm = co_run->rpm;
    S_CO_RUN_STEP *step = co_run->stepptr[stepindex];

    if (stepindex >= co_run->numofstep) {
        return -1;
    }

    //calculate speed;
    int16 acc;
    if (step->speed != NULL) {
        int32 tarrpm = step->speed->rpm;
        uint32 ramp = step->speed->ramp[size];
        acc = (tarrpm - (int32)co_run->prerpm) / (int32)ramp;
        co_run->speedAcc = acc;
        co_run->targetSpeed = tarrpm;
    }

    co_run->rpm = co_run->prerpm + co_run->speedAcc;
    if (co_run->speedAcc < 0 && co_run->rpm < co_run->targetSpeed) {
        co_run->rpm = co_run->targetSpeed;
    } else if (co_run->speedAcc > 0 && co_run->rpm > co_run->targetSpeed) {
        co_run->rpm = co_run->targetSpeed;
    }
    if (co_run->rpm == co_run->targetSpeed) {
        co_run->speedAcc = 0;
    }

    //calculate sizemotor;
    SIZEMOTOR_ZONE *sizemotor_zone = step->sizemotor;
    if (sizemotor_zone != NULL) {
        line->zonename = step->sizemotor->descrpition;
        line->zonebegin = step->sizemotor->head.beginStep;
        line->zoneend = step->sizemotor->head.endStep;
        uint32 sizemotorbase = step->sizemotor->param[size].start;
        int32 sizemotoracc = step->sizemotor->param[size].acc;
        uint32 sizemotorend = step->sizemotor->param[size].end;
        uint32 sizemotorbaseline = co_run->stepptr[step->sizemotor->head.beginStep]->ilinetag[size];
        //uint32 sizemotorendline = co_run->stepptr[step->sizemotor->head.endStep]->ilinetag;
        co_run->sizemotor = sizemotorbase * 1000 + sizemotoracc * (co_run->nextline - sizemotorbaseline);
        if ((sizemotoracc > 0 && co_run->sizemotor > sizemotorend)
            || (sizemotoracc < 0 && co_run->sizemotor < sizemotorend)) {
            co_run->sizemotor = sizemotorend;
        }
    } else {
        line->zonename = NULL;
    }

    //welt;
    line->welt = co_run->welt = step->welt;

    //funcode
    funcodeParse(step->func, co_run->act);

    //process step
    if (IS_ECONO_BEGIN(*step)) { // loop begin
        co_run->iecono++;
        co_run->econonum = step->econo->economize[size];
        co_run->econostepfrom = step->econo->begin;
        co_run->econostepto = step->econo->end;
    }
    if (IS_ECONO_END(*step)) { //loop end
        if (co_run->iecono == co_run->econonum) { //end loop
            co_run->nextstep++;
            co_run->iecono = 0;
            co_run->econostepfrom = 0;
            co_run->econostepto = 0;
            co_run->econonum = 0;
        } else {
            co_run->nextstep = co_run->econostepfrom;
        }
    } else {
        co_run->nextstep++;
    }

    co_run->nextline++;

    line->rpm = co_run->rpm;
    line->econonum = co_run->econonum;
    line->iecono = co_run->iecono;
    line->econobegin = co_run->econostepfrom;
    line->econoend = co_run->econostepto;

    line->sizemotor = co_run->sizemotor;

    line->iline = co_run->nextline - 1;
    line->istep = stepindex;

    line->act = co_run->act;

    return co_run->numofline[size] - line->iline - 1;
}


uint32 corunReadStep(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size) {
    uint32 stepindex = co_run->istep = co_run->nextstep;
    co_run->prerpm = co_run->rpm;
    S_CO_RUN_STEP *step = co_run->stepptr[stepindex];

    if (stepindex >= co_run->numofstep) {
        return -1;
    }

    //calculate speed;
    int16 acc;
    if (step->speed != NULL) {
        int32 tarrpm = step->speed->rpm;
        uint32 ramp = step->speed->ramp[size];
        acc = (tarrpm - (int32)co_run->prerpm) / (int32)ramp;
        co_run->speedAcc = acc;
        co_run->targetSpeed = tarrpm;
    }

    co_run->rpm = co_run->prerpm + co_run->speedAcc;
    if (co_run->speedAcc < 0 && co_run->rpm < co_run->targetSpeed) {
        co_run->rpm = co_run->targetSpeed;
    } else if (co_run->speedAcc > 0 && co_run->rpm > co_run->targetSpeed) {
        co_run->rpm = co_run->targetSpeed;
    }
    if (co_run->rpm == co_run->targetSpeed) {
        co_run->speedAcc = 0;
    }

    //calculate sizemotor;
    if (step->sizemotor != NULL) {
        line->zonename = step->sizemotor->descrpition;
        line->zonebegin = step->sizemotor->head.beginStep;
        line->zoneend = step->sizemotor->head.endStep;
    } else {
        line->zonename = NULL;
    }

    //welt;
    line->welt = co_run->welt = step->welt;

    //funcode
    funcodeParse(step->func, co_run->act);

    //process step
    co_run->nextstep++;
    co_run->nextline = step->ilinetag[size] + 1;

    //set line data;
    line->rpm = co_run->rpm;
    line->econonum = co_run->econonum;
    line->iecono = co_run->iecono;
    line->econobegin = co_run->econostepfrom;
    line->econoend = co_run->econostepto;
    line->iline = co_run->nextline - 1;
    line->istep = stepindex;

    line->act = co_run->act;

    return co_run->numofline[size] - line->iline - 1;
}



bool corunSeekLine(S_CO_RUN *co_run, uint32 line, uint32 size) {
    if (line >= co_run->numofline[size]) {
        return false;
    }
    //seek 0
    co_run->istep = 0;                   //step conter when run;   			//当前STEP
    co_run->nextline = 0;                                                //下一行
    co_run->nextstep = 0;               //nextstep != istep+1, due to economizer
    co_run->prerpm = 0;
    co_run->rpm = 0;
    co_run->iecono = 0;                  //current economizer counter
    co_run->econonum = 0;                //economizer
    co_run->econostepfrom = 0;           //economizer begin step
    co_run->econostepto = 0;             //economizer end step(end>begin)
    co_run->speedAcc = 0;                //speed acceleration when run
    co_run->targetSpeed = 0;             //target speed when ramp
                                         //seek line;
    S_CO_RUN_LINE dummyline;
    for (int i = 0; i < line; i++) {
        corunReadLine(co_run, &dummyline, size);
    }
    return true;
}


typedef __packed struct {
    char co[12];
    short unknow;
    uint32 product;
    char dummy[12];
}
CN_GROUP;



void createCn(const TCHAR *path, S_CN_GROUP *co) {


}



bool cnParse(const TCHAR *path, S_CN_GROUP *val) {
    //open co file
    FIL file;
    uint32 br;
    CN_GROUP cogp;
    FRESULT r = f_open(&file, path, FA_READ);
    if (r != FR_OK) {
        //re = CO_FILE_OPEN_ERROR;
        return false;
    }
    if (f_size(&file) != 512) {
        goto ERROR;
    }
    f_lseek(&file, 0x16a);

    for (int i = 0; i < 5; i++) {
        r = f_read(&file, &cogp, sizeof cogp,&br);
        if (r != FR_OK && br != sizeof cogp) {
            goto ERROR;
        }
        cogp.co[8] = 0;
        for (int j = 7; j > 0; j--) {
            if (cogp.co[j] == 0x20) {
                cogp.co[j] = 0;
            } else {
                break;
            }
        }
        if (cogp.co[0] != 0) {
            strcpy(val[i].filename, cogp.co);
            strcat(val[i].filename, ".co");
            val[i].num = cogp.product;
        } else {
            val[i].filename[0] = 0;
            val[i].num = 0;
        }
    }
    f_close(&file);
    return true;
    ERROR:
    f_close(&file);
    return false;
}



static void funcode2Valvecode(FUNC *fun, uint16 *valvecode, uint32 *num);

static void funcodeParse(struct list_head *func, ACT_GROUP *angleValve) {
    struct list_head *p;
    for (int i = 0; i < 360; i++) {
        angleValve[i].num = 0; //cear ANGLE_VALVE::num ,  ANGLE_VALVE::inum
    }
    if (func == NULL) {
        return;
    }
    list_for_each(p, func) {
        FUNC *fun = list_entry(p, FUNC, list);
        uint32 angle = fun->angular;
        uint32 valvecodenum;
        funcode2Valvecode(fun, &angleValve[angle].valvecode[angleValve[angle].num], &valvecodenum);
        angleValve[angle].num += valvecodenum;
    }
}


static uint16 freeselcode2Valvecode(uint16 funcval) {
    uint16 valvecode;
    uint32 line, feed;
    if (funcval >= 0x65 && funcval <= 0xa4) { //free sel line 1 to line8
        valvecode = !((funcval - 0x65) % 2) << 12;  //in:1<<12  out:0<<12
        line = (funcval - 0x65) / 8;
        feed = (funcval - 0x65) / 2 % 4;
        valvecode |= SEL_LINE_NUMBER * feed + line + SEL_BASE;
    } else if (funcval >= 0xc5 && funcval <= 0x104) { //free sel line 9 to line 16
        valvecode = (!(funcval - 0xc5) % 2) << 12;  //in:1<<12  out:0<<12
        line = (funcval - 0xc5) / 8 + 8;
        feed = (funcval - 0xc5) / 2 % 4;
        valvecode |= SEL_LINE_NUMBER * feed + line + SEL_BASE;
    }
    return valvecode;
}




static void fixselcode2Valvecode(uint16 funcval, uint16 *valvecode, uint32 *num) {
    uint32 feed;
    if (funcval >= 0x1d && funcval <= 0x24) { //1x1i    sel line 16
        feed = (funcval - 0x1d) / 2;
        *valvecode = !((funcval - 0x1d) % 2) << 12;
        *valvecode |= SEL_LINE_NUMBER * feed + 15 + SEL_BASE;
        *num = 1;
    } else if (funcval >= 0x25 && funcval <= 0x2c) { //MICRO
        *num = 0;
    } else if (funcval >= 0x2d && funcval <= 0x34) { //1X1O    sel line 14 15
        feed = (funcval - 0x2d) / 2;
        uint16 inout = !((funcval - 0x2d) % 2) << 12;
        valvecode[0] = valvecode[1] = inout;
        valvecode[0] |= SEL_LINE_NUMBER * feed + 14 + SEL_BASE;
        valvecode[1] |= SEL_LINE_NUMBER * feed + 13 + SEL_BASE;
        *num = 2;
    } else if (funcval >= 0x35 && funcval <= 0x3c) { //3X1i    sel line 14
        feed = (funcval - 0x35) / 2;
        *valvecode = !((funcval - 0x35) % 2) << 12;
        *valvecode |= SEL_LINE_NUMBER * feed + 13 + SEL_BASE;
        *num = 1;
    } else if (funcval >= 0x3d && funcval <= 0x44) { //3X1o    sel line 15
        feed = (funcval - 0x3d) / 2;
        *valvecode = !((funcval - 0x3d) % 2) << 12;
        *valvecode |= SEL_LINE_NUMBER * feed + 14 + SEL_BASE;
        *num = 1;
    } else if (funcval >= 0x45 && funcval <= 0x4c) { //1x3    sel line 14 16
        feed = (funcval - 0x45) / 2;
        uint16 inout = !((funcval - 0x45) % 2) << 12;
        valvecode[0] = valvecode[1] = inout;
        valvecode[0] |= SEL_LINE_NUMBER * feed + 15 + SEL_BASE;
        valvecode[1] |= SEL_LINE_NUMBER * feed + 13 + SEL_BASE;
        *num = 2;
    } else if (funcval >= 0x4d && funcval <= 0x54) { //N-N
        *num = 0;
    } else if (funcval >= 0x55 && funcval <= 0x5c) { //O-O  sel line 12
        feed = (funcval - 0x55) / 2;
        *valvecode = !((funcval - 0x55) % 2) << 12;
        *valvecode |= SEL_LINE_NUMBER * feed + 11 + SEL_BASE;
        *num = 1;
    } else if (funcval >= 0x5D && funcval <= 0x64) { //T-BAND  sel line 12
        feed = (funcval - 0x5D) / 2;
        *valvecode = !((funcval - 0x5D) % 2) << 12;
        *valvecode |= SEL_LINE_NUMBER * feed + 11 + SEL_BASE;
        *num = 1;
    } else { //1X1a 3X1a DIASX DIADX
        *num = 0;
    }
}

static uint16 hafuzhencode2Valvecode(uint16 funcval) {
    /*uint32 phase = (funcval-0x0d)/2%2;
    uint32 inorout = !((funcval-0x0d)%2);
    uint32 enterexit = (funcval-0x0d)/4;
    uint16 valvecode = inorout<<12;
    valvecode |= HAFUZHEN_BALSE*/
}


static uint16 camcode2Valvecode(FUNC *fun) {
    uint32 pos_ace = fun->value % 3;
    uint32 sxt = fun->value / 3 % 3;
    uint32 feed = fun->value / 9;
}

static uint16 yfingercode2Valvecode(uint16 codevalue) {
    //YARN_FINGER_BASE
}

static uint16 misc0203code2Valvecode(uint16 codevalue) {
    int16 inorout;
    int16 ivalve;
    int16 re;
    if (codevalue <= 0x47) {
        inorout = !(codevalue % 2) << 12;
        ivalve = codevalue >> 2;
        re = inorout | ivalve + VALVE_MIS_BASE;
    } else if (codevalue >= 0x98 && codevalue <= 0xa9) {
        inorout = !(codevalue % 2) << 12;
        ivalve = codevalue - (0x98 - 0x48) >> 2;
        re = inorout | ivalve + VALVE_MIS_BASE;
    } else if (codevalue >= 0xb4 && codevalue <= 0xc3) {
        inorout = !(codevalue % 2) << 12;
        ivalve = codevalue - (0xb4 - 0xaa) - (0x98 - 0x48) >> 2;
        re = inorout | ivalve + VALVE_MIS_BASE;
    }
    return re;
}

static void funcode2Valvecode(FUNC *fun, uint16 *valvecode, uint32 *num) {
    //uint16 val;
    *num = 0;
    switch (fun->funcode) {
    case 0x031e: //sel
        if (fun->value >= 0x0d && fun->value <= 0x14) { //hafu zhen sanjiao
            *valvecode = hafuzhencode2Valvecode(fun->funcode);
            *num = 1;
        } else if ((fun->value >= 0x65 && fun->value <= 0xa4)
                   || (fun->value >= 0xc5 && fun->value <= 0x104)) { //free sel line 1 to line8
            *valvecode = freeselcode2Valvecode(fun->value);
            *num = 1;
        } else if ((fun->value >= 0x1d && fun->value <= 0x64)
                   || (fun->value >= 0xa5 && fun->value <= 0xc4)) { // fix sel
            fixselcode2Valvecode(fun->value, valvecode, num);
        }
        break;
    case 0x0305:  //cam
        //camcode2Valvecode(FUNC * fun);
        break;
    }
}








