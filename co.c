#include "type.h"
#include "list.h"
#include "ff.h"
#include <string.h>
#include "pf_platform_cfg.h"
#include "mmath.h"
#include "co.h"
#include "algorithm.h"
#include "debug.h"
#include <wchar.h>
#include "ff_ext.h"
#include <stdlib.h>
#include "mem.h"
#include <stdio.h>
#include "misc.h"
#include "qifacode.h"


#define MOTOR_SCALE   1
#define FILEPATH  L"1:\\"
#define BTSR_DIS_BUF_SIZE 512*1024
#define BTSR_DIS_BUF_CNT  6





static FUNC __func[2000];
static struct list_head funcfreelist;

static SPEED __speed[800];
static struct list_head speedfreelist;

static FENGMEN __fengmen[1000];
static struct list_head fengmenfreelist;

static S_CO_RUN_STEP __run_step[3000];
static struct list_head runstepfreelist;

static WELT_PARAM __welt_param_pool[100];
static struct list_head welt_param_freelist;

static BTSR __btsr_pool[10];
static struct list_head btsr_freelist;
static struct list_head btsrlist;


static unsigned char cofilebuf[1024 * 2048];

//static ACT_GROUP act360[360];

//static ALARM_GROUP alarm360[360];


static uint32 get_co_check(void *buf, uint32 dwSize) {
    ASSERT(dwSize % 4 == 0);
    int i, j = 0;
    uint32 *buf32;
    int x2;
    uint32 aVal = 0;
    uint32 bVal;
    buf32 = (uint32 *)buf;
    while (1) {
        bVal = buf32[j++];
        for (i = 0; i < 32; i++) {
            x2 = (aVal & 0x80000000) ^ (bVal & 0x80000000);
            if (x2 == 0x80000000) {
                aVal = aVal ^ 0x84c11db7;
            }
            aVal <<= 1;
            aVal += 1;
            bVal <<= 1;
        }
        if (j >= dwSize / 4) break;
    }
    return aVal;
}



static OS_MEM jacmem,btsr_dis_mem,guidmem;


MACHINE machine;

static void L10L12_fun0203Resolve(uint16 codevalue, uint16 *valvecode, uint32 *valnum);
static void L10L12_031eToValvecode(FUNC *fun, uint16 *valvecode, uint32 *valnum,
                                   uint16 *alarmcode, uint32 *alarmnum);
static void L10L12_funcode2Alarm(FUNC *func, uint16 *alarmcode, uint32 *alarmnum);


static void L04E7_fun0203Resolve(uint16 codevalue, uint16 *valvecode, uint32 *valnum);
static void L04E7_031eToValvecode(FUNC *fun, uint16 *valvecode, uint32 *valnum,
                                  uint16 *alarmcode, uint32 *alarmnum);
static void L04E7_funcode2Alarm(FUNC *func, uint16 *alarmcode, uint32 *alarmnum);


void coInit(char machinename[], uint32 niddleNum, uint32 sel_PreNiddleNum) {
    INIT_LIST_HEAD(&funcfreelist);
    INIT_LIST_HEAD(&fengmenfreelist);
    INIT_LIST_HEAD(&runstepfreelist);
    INIT_LIST_HEAD(&welt_param_freelist);
    INIT_LIST_HEAD(&speedfreelist);
    INIT_LIST_HEAD(&btsr_freelist);
    INIT_LIST_HEAD(&btsrlist);

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
    for (int i = 0; i < lenthof(__speed); i++) {
        list_add_tail(&__speed[i].list, &speedfreelist);
    }
    for (int i = 0; i < lenthof(__btsr_pool); i++) {
        list_add_tail(&__btsr_pool[i].list, &btsr_freelist);
    }
    //creat jacq memery pool
    static SEL_JAC jacpool[1000];
    static SEL_GUID guidpool[30];
    static char btsr_dis_buf[BTSR_DIS_BUF_CNT][BTSR_DIS_BUF_SIZE];
    MEM_ERR memerr;
    MemCreate(&jacmem, "jacq memery pool", jacpool, lenthof(jacpool), sizeof(SEL_JAC), &memerr);
    ASSERT(memerr == MEM_ERR_NONE);
    MemCreate(&guidmem, "guid memery pool", guidpool, lenthof(guidpool), sizeof(SEL_GUID), &memerr);
    ASSERT(memerr == MEM_ERR_NONE);
    MemCreate(&btsr_dis_mem, "btsr dis memery", btsr_dis_buf, BTSR_DIS_BUF_CNT, BTSR_DIS_BUF_SIZE, &memerr);
    ASSERT(memerr == MEM_ERR_NONE);
    ASSERT(niddleNum % 16 == 0);
    machine.niddleNum = niddleNum;
    machine.feedNum = 4;
    machine.selPreNiddleNum = sel_PreNiddleNum;
    strcpy(machine.name, machinename);
    if (strcmp(machinename, "L510") == 0) {
        machine.fun0203Resolve = L10L12_fun0203Resolve;
        machine.fun031eToValvecode = L10L12_031eToValvecode;
        machine.funcode2Alarm = L10L12_funcode2Alarm;
    } else if (strcmp(machinename, "L04E7") == 0) {
        machine.fun0203Resolve = L04E7_fun0203Resolve;
        machine.fun031eToValvecode = L04E7_031eToValvecode;
        machine.funcode2Alarm = L04E7_funcode2Alarm;
    } else {
        while (1);
    }
}



static void readSinkMotor(void *sinkmotorbuf, SINKERMOTOR_ZONE *zone, unsigned int *num) {
    *num = 0;
    for (int i = 0;; i++) {
        CO_SINKERMOTOR_ZONE *co_zone = (CO_SINKERMOTOR_ZONE *)sinkmotorbuf + i;
        if (co_zone->size != 0) {
            SINKERMOTOR_ZONE *zonetemp = zone + *num;

            memcpy(&zonetemp->descrpition, &co_zone->descrpition, sizeof zonetemp->descrpition);

            zonetemp->head.angle = co_zone->head.angle;
            zonetemp->head.beginStep = co_zone->head.beginStep;
            zonetemp->head.endStep = co_zone->head.endStep;
            zonetemp->head.groupNum = co_zone->head.groupNum;

            for (int i = 0; i < 8; i++) {
                zonetemp->param[i].qf_feed = co_zone->param[i].qf_feed[0] * MOTOR_SCALE;
                zonetemp->param[i].qi_feed = co_zone->param[i].qi_feed[0] * MOTOR_SCALE;
            }
            (*num)++;
        } else {
            break;
            //TODO: numof sizemotor zone protect
        }
    }
}



static void readSizemotor(uint8 *sizemotorbuf, SIZEMOTOR_ZONE *zone, unsigned int *num) {
    *num = 0;
    for (int i = 0;; i++) {
        CO_SIZEMOTOR_ZONE *co_szone = (CO_SIZEMOTOR_ZONE *)sizemotorbuf + i;
        if (co_szone->size != 0) {
            SIZEMOTOR_ZONE *zone_temp = zone + *num;
            memcpy(&zone_temp->descrpition, &co_szone->descrpition, sizeof zone->descrpition);

            zone_temp->head.angle = co_szone->head.angle;
            zone_temp->head.beginStep = co_szone->head.beginStep;
            zone_temp->head.endStep = co_szone->head.endStep;
            zone_temp->head.groupNum = co_szone->head.groupNum;

            for (int i = 0; i < 8; i++) {
                zone_temp->param[i].start = co_szone->param[i].start[0] * MOTOR_SCALE;
                zone_temp->param[i].startWidth = co_szone->param[i].startWidth;
                zone_temp->param[i].startWidthDec = co_szone->param[i].startWidthDec;
                zone_temp->param[i].end = co_szone->param[i].end[0] * MOTOR_SCALE;
                zone_temp->param[i].endWidth = co_szone->param[i].endWidth;
                zone_temp->param[i].endWidthDec = co_szone->param[i].endWidthDec;
            }
            (*num)++;
        } else {
            break;
            //TODO: numof sizemotor zone protect
        }
    }
}


static void readSpeed(uint8 *speedbuf, struct list_head *speed) {
    for (int i = 0;; i++) {
        CO_SPEED *co_speed = (CO_SPEED *)speedbuf + i;
        if (co_speed->step != 0xffffffff) {
            SPEED *speedtemp = list_first_entry_or_null(&speedfreelist, SPEED, list);
            ASSERT(speedtemp != NULL);
            list_del(&speedtemp->list);
            for (int i = 0; i < 8; i++) {
                speedtemp->ramp[i] = co_speed->ramp[i][0];
                ASSERT(co_speed->ramp[i][0] == co_speed->ramp[i][1]);
            }
            ASSERT(co_speed->rpm[0] == co_speed->rpm[1]);
            speedtemp->rpm = co_speed->rpm[0];
            speedtemp->step = co_speed->step;
            speedtemp->coed = true;
            list_add_tail(&speedtemp->list, speed);
        } else {
            break;
            //TODO: numof SPEED protect
        }
    }
}


bool readFunc(uint8 *funcaddr, struct list_head *funclist, unsigned int *step) {
    *step = *(uint32 *)funcaddr / 4;
    for (int i = 0; i < *step; i++) {
        uint32 addr = ((uint32 *)funcaddr)[i];
        uint32 addrnext = ((uint32 *)funcaddr)[i + 1];
        //read func;
        uint32 addradd;
        while (1) {
            FUNC *func = list_first_entry_or_null(&funcfreelist, FUNC, list);
            ASSERT(func != NULL);
            CO_FUNC *cofunc = (CO_FUNC *)(funcaddr + addr);
            if (cofunc->angular != 0x8000) {
                ASSERT(cofunc->angular < 360);
                addr += 6;
                if ((cofunc->funcode == 0x031f) || (cofunc->funcode == 0x011f)) {
                    addradd = 4;
                } else if (cofunc->funcode == 0x030b) {
                    addradd = 2;
                } else {
                    addradd = 0;
                }
                addr += addradd;
                func->angular = cofunc->angular;
                func->value = cofunc->value;
                func->funcode = cofunc->funcode;
                func->funcode = cofunc->funcode;
                if (addradd == 2) {
                    *(uint32 *)func->add = *(short *)cofunc->add;
                } else if (addradd == 2) {
                    *(uint32 *)func->add = *(uint32 *)cofunc->add;
                }
                list_move_tail(&func->list, &funclist[i]);
            } else {
                if ((i != *step - 1) && (addrnext != addr + 2)) { //is not last step
                    return false;
                }
                break;
            }
        }
    }
    return true;
}


static void readFengmen(uint8 *fengmenbuf, struct list_head *fengmenlist, unsigned int *step) {
    *step = *(uint32 *)fengmenbuf / 4;
    for (int i = 0; i < *step; i++) {
        FENGMEN *fengmen;
        CO_FENGMEN *fengmentemp = (CO_FENGMEN *)(fengmenbuf + *((uint32 *)fengmenbuf + i));
        while (1) {
            fengmen = list_first_entry_or_null(&fengmenfreelist, FENGMEN, list);
            ASSERT(fengmen != NULL);
            if (fengmentemp->angular != 0x8000) {
                ASSERT(fengmentemp->angular < 360);
                fengmen->angular = fengmentemp->angular;
                fengmen->dummy = fengmentemp->dummy;
                fengmen->funcode = fengmentemp->funcode;
                fengmen->val = fengmentemp->val;
                list_move_tail(&fengmen->list, &fengmenlist[i]);
                fengmentemp++;
            } else {
                break;
            }
        }
    }
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


static void readWelt(uint8 *weltbuf, struct list_head *weltListHead) {
    CO_WELT_PARAM *co_welt;
    for (int i = 0;; i++) {
        co_welt = (CO_WELT_PARAM *)weltbuf + i;
        ASSERT(co_welt->weltflag == 0x2042 || co_welt->weltflag == 0x2062
               || co_welt->weltflag == 0x00);
        WELT_PARAM *welt;
        if (co_welt->weltflag == 0x2042) { //welt in
            welt = list_entry(welt_param_freelist.next, WELT_PARAM, list);
            welt->weltinstep = co_welt->step;
            list_move_tail(welt_param_freelist.next, weltListHead);
        } else if (co_welt->weltflag == 0x2062) { //welt out
            welt->weltoutstep = co_welt->step - 1;
        } else if (co_welt->weltflag == 0x00) {
            break;
        }
    }
}


static void readEconomizer(uint8 *econobuf, ECONOMIZER_PARAM *econo, uint32 *num) {
    *num = 0;
    CO_ECONOMIZER_PARAM *param;
    for (int i = 0;; i++) {
        param = (CO_ECONOMIZER_PARAM *)econobuf + i;
        if (param->begin == 0 && param->end == 0
            && param->economize[0][0] != param->economize[0][1]) {
            break;
        }
        econo[i].begin = param->begin;
        econo[i].end = param->end;
        for (int j = 0; j < 8; j++) {
            econo[i].economize[j] = param->economize[j][0];
        }
        (*num)++;
    }
}


#define MACHINE_NIDDLE_NUM_NOT_MATCH   -1

static int32 co_read_reset(S_CO *co, uint8 resetpartbuf[], bool checkNiddleNum) {
    CO_RESET_INFO *info = (CO_RESET_INFO *)resetpartbuf;

    memcpy(&co->resetinfo, info, sizeof*info);

    co->diameter = info->diameter;
    if (checkNiddleNum && info->niddle != machine.niddleNum) {
        return MACHINE_NIDDLE_NUM_NOT_MATCH;
    }
    co->niddle = info->niddle;


    //read sizemoter;
    readSizemotor(resetpartbuf + info->sizemotorAddr, co->sizemotor,
                  &co->numofsizemotorzone);

    //read sinkermotor_feed1_3;
    readSinkMotor(resetpartbuf + info->sinkermotor_feed1_3_Addr,
                  co->sinkmoterzone_1_3, &co->numofsinkmoterzone_1_3);

    //read sinkermotor_feed2_4;
    if (info->sinkermotor_feed2_4_Addr != 0) {
        readSinkMotor(resetpartbuf + info->sinkermotor_feed2_4_Addr,
                      co->sinkmoterzone_2_4, &co->numofsinkmoterzone_2_4);
    }

    //read sinkerangular;
    if (info->sinkerangular_Addr != 0) {
        readSinkMotor(resetpartbuf + info->sinkerangular_Addr,
                      co->sinkangular, &co->numofsinkangular);
    }
    return 0;
}


static void co_read_yarn(S_CO *co, uint8 yarnpartbuf[]) {
    /*CO_PART2_ATTRIB *coattri2 = (CO_PART2_ATTRIB *)buf;
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
    }*/
}

static bool co_read_cate(S_CO *co, uint8 catepartbuf[]) {
    CO_CATE_INFO *info = (CO_CATE_INFO *)catepartbuf;

    memcpy(&co->cateinfo, info, sizeof*info);

    readSpeed(catepartbuf + info->speed_addr, &co->speed);

    unsigned int step;
    bool r = readFunc(catepartbuf + info->func_addr, co->func, &step);
    co->numofstep = step;
    return r;
}

static bool co_read_mpp(S_CO *co, uint8 mpppartbuf[]) {
    CO_MPP_INFO *mppinfo = (CO_MPP_INFO *)mpppartbuf;
    uint32 step;
    readFengmen(mpppartbuf + mppinfo->fengmen1addr, co->fengmen, &step);
    if (step != co->numofstep) {
        return false;
    }
    return true;
}


static void co_read_supe(S_CO *co, uint8 supepartbuf[]) {
    CO_SUPE_INFO *supeinfo = (CO_SUPE_INFO *)supepartbuf;

    memcpy(&co->supeinfo, supeinfo, sizeof*supeinfo);

//read welt;
    readWelt(supepartbuf + supeinfo->weltAddrOffset, &co->welt);

//read economizers
    readEconomizer(supepartbuf + supeinfo->econoAddrOffset, co->econo, &co->numofeconomizer);
}




static unsigned int co_read_jacq(S_CO *co, uint8 jacpcrypteddata[], unsigned int size) {
    unsigned int desize = coDecrypteSize(jacpcrypteddata);
    unsigned char *p = malloc(desize);
    ASSERT(p != NULL);
    coDecrypt(jacpcrypteddata, size, p);
    co->jacsize = desize;
    co->jac = p;
    return desize;
}


static unsigned int co_read_dis(S_CO *co, uint8 discrypteddata[], unsigned int size) {
    unsigned int desize = coDecrypteSize(discrypteddata);
    unsigned char *p = malloc(desize);
    ASSERT(p != NULL);
    coDecrypt(discrypteddata, size, p);
    co->dissize = desize;
    co->dis = p;
    return desize;
}

static unsigned int co_read_guid(S_CO *co, uint8 guidcrypted[], unsigned int size) {
    unsigned int desize = coDecrypteSize(guidcrypted);
    unsigned char *p = malloc(desize);
    ASSERT(p != NULL);
    coDecrypt(guidcrypted, size, p);
    co->guidsize = desize;
    co->guid = p;
    return desize;
}


uint32 co_get_crc(void *dat, uint16 dwSize) {

    int i, j = 0;
    uint32 x;
    int x2;
    uint32 aVal = 0;
    uint32 bVal;
    //uint32 len = dwSize;
    uint32 temp;

    uint8 *buf = (uint8 *)dat;
    while (1) {
        x = buf[j];
        temp = buf[1 + j];
        x += temp << 8;
        temp = buf[2 + j];
        x += temp << 16;
        temp = buf[3 + j];
        x += temp << 24;

        bVal = x;

        for (i = 0; i < 32; i++) {
            x2 = (aVal & 0x80000000) ^ (bVal & 0x80000000);

            if (x2 == 0x80000000) {
                aVal = aVal ^ 0x84c11db7;
            }
            aVal <<= 1;
            aVal += 1;
            bVal <<= 1;
        }

        j += 4;

        if (j >= dwSize) break;
    }
    //x = (aVal & 0xff) << 24;
    //x = x + ((aVal & 0xff00) << 8);
    //x = x + ((aVal & 0xff000000) >> 24);
    //x = x + ((aVal & 0xff0000) >> 8);
    return aVal;
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
}


int32 coParse(const TCHAR *path, S_CO *co, uint32 flag, unsigned int *offset) {
    FIL file;
    unsigned int br;
    int32 re = CO_FILE_READ_OK;

    if (co->parsed) {
        return CO_PARSE_PARSED_ERROR;
    }

    //do co struct init
    for (int i = 0; i < lenthof(co->func); i++) {
        INIT_LIST_HEAD(&co->func[i]);
    }
    for (int i = 0; i < lenthof(co->fengmen); i++) {
        INIT_LIST_HEAD(&co->fengmen[i]);
    }
    INIT_LIST_HEAD(&co->speed);
    INIT_LIST_HEAD(&co->welt);

    char temp1[20], temp2[20];
    unsigned int dissize, jacqsize, guidsize;

    //open co file
    FRESULT r = f_open(&file, path, FA_READ);
    if (r != FR_OK) {
        re = CO_FILE_READ_ERROR;
        return re;
    }

    wpathTowfilename(co->filename, path);

    //READ CO HEAD
    r = f_read(&file, &co->head, sizeof(CO_HEADER), &br);
    if (r != FR_OK || br != sizeof(CO_HEADER)) {
        re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    strtrim(temp1, co->head.machineName);
    strtrim(temp2, machine.name);
    if (strcmp(temp1, temp2) != 0) {
        re = CO_MACHINE_NOT_MATCH;
        goto ERROR;
    }
    //unsigned char ver = buf[28];
    if (co->head.coCheck != get_co_check((uint8 *)&co->head + 4, sizeof(CO_HEADER) - 4)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }

    //READ CO RESET SECTION(section0)
    f_lseek(&file, co->head.sec[0].offset << 8);
    r = f_read(&file, &cofilebuf, co->head.sec[0].len << 8, &br);
    if (r != FR_OK || br != co->head.sec[0].len << 8) {
        re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (co->head.sec[0].check != get_co_check(cofilebuf, co->head.sec[0].len << 8)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }
    if (co_read_reset(co, cofilebuf, (flag & 0x01)) == MACHINE_NIDDLE_NUM_NOT_MATCH) {
        re = CO_NIDDLE_NOT_MATCH;
        goto ERROR;
    }

    //READ CO YARN SECTION(section 1)
    f_lseek(&file, co->head.sec[1].offset << 8);
    r = f_read(&file, &cofilebuf, co->head.sec[1].len << 8, &br);
    if (r != FR_OK || br != co->head.sec[1].len << 8) {
        re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (co->head.sec[1].check != get_co_check(cofilebuf, co->head.sec[1].len << 8)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }
    co_read_yarn(co, cofilebuf);

    //READ CO CATE SECTION(section 2)
    f_lseek(&file, co->head.sec[2].offset << 8);
    r = f_read(&file, &cofilebuf, co->head.sec[2].len << 8, &br);
    if (r != FR_OK || br != co->head.sec[2].len << 8) {
        re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (co->head.sec[2].check != get_co_check(cofilebuf, co->head.sec[2].len << 8)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }
    if (!co_read_cate(co, cofilebuf)) {
        re = CO_FILE_PARSE_ERROR;
        goto ERROR;
    }

    //READ CO MPP SECTION(section 3)
    f_lseek(&file, co->head.sec[3].offset << 8);
    r = f_read(&file, &cofilebuf, co->head.sec[3].len << 8, &br);
    if (r != FR_OK || br != co->head.sec[3].len << 8) {
        re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (co->head.sec[3].check != get_co_check(cofilebuf, co->head.sec[3].len << 8)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }
    if (!co_read_mpp(co, cofilebuf)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }
    //READ CO JACQ SECTION(section 4)
    f_lseek(&file, co->head.sec[4].offset << 8);
    jacqsize = co->head.sec[4].len << 8;
    r = f_read(&file, &cofilebuf, jacqsize, &br);
    if (r != FR_OK || br != jacqsize) {
        re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (co->head.sec[4].check != get_co_check(cofilebuf, jacqsize)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }
    co_read_jacq(co, cofilebuf, jacqsize);

    //read co GUID section

    f_lseek(&file, co->head.sec[5].offset << 8);
    guidsize = co->head.sec[5].len << 8;
    r = f_read(&file, &cofilebuf, guidsize, &br);
    if (r != FR_OK || br != guidsize) {
        re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (co->head.sec[5].check != get_co_check(cofilebuf, guidsize)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }
    co_read_guid(co, cofilebuf, guidsize);


    //READ CO DIS SECTION(section 6)
    f_lseek(&file, co->head.sec[6].offset << 8);
    dissize = co->head.sec[6].len << 8;
    r = f_read(&file, &cofilebuf, dissize, &br);
    if (r != FR_OK || br != dissize) {
        re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (co->head.sec[6].check != get_co_check(cofilebuf, dissize)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }
    co_read_dis(co, cofilebuf, dissize);


    //READ CO SUPE SECTION(section 7)
    f_lseek(&file, co->head.sec[7].offset << 8);
    r = f_read(&file, &cofilebuf, co->head.sec[7].len << 8, &br);
    if (r != FR_OK || br != co->head.sec[7].len << 8) {
        re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (co->head.sec[7].check != get_co_check(cofilebuf, co->head.sec[7].len << 8)) {
        re = CO_FILE_CHECK_ERROR;
        goto ERROR;
    }
    co_read_supe(co, cofilebuf);


    f_close(&file);
    co->machine = &machine;
    co->parsed = true;
    return re;

    ERROR:
    if (offset != NULL) {
        *offset = (uint32)f_tell(&file);
    }
    f_close(&file);
    co->parsed = false;
    return re;
}


static void freeBtsr(S_CO_RUN *co_run);


void stepJacqRelease(S_CO_RUN *co_run) {
    for (int i = 0; i < co_run->seljacnum; i++) {
        MEM_ERR err;
        MemPut(&jacmem, co_run->seljac[i], &err);
    }
    co_run->seljacnum = 0;
}


void stepGuidRelease(S_CO_RUN *co_run) {
    for (int i = 0; i < co_run->selguidnum; i++) {
        MEM_ERR err;
        MemPut(&guidmem, co_run->selguid[i], &err);
    }
    co_run->selguidnum = 0;
}

void coRelease(S_CO *co) {
    if (!co->parsed) {
        return;
    }
    co->parsed = 0;
    struct list_head *p,*n;
    list_for_each_safe(p, n, &co->speed) {
        SPEED *speed = list_entry(p, SPEED, list);
        list_del(p);
        memset(speed, 0, sizeof(SPEED));
        list_add(p, &speedfreelist);
    }


    FUNC *func;
    for (int i = 0; i < lenthof(co->func); i++) {
        list_for_each_safe(p, n, &co->func[i]) {
            func = list_entry(p, FUNC, list);
            list_del(p);
            memset(func, 0, sizeof(FUNC));
            list_add(p, &funcfreelist);
        }
    }
    FENGMEN *fengmen;
    for (int i = 0; i < lenthof(co->fengmen); i++) {
        list_for_each_safe(p, n, &co->fengmen[i]) {
            fengmen = list_entry(p, FENGMEN, list);
            list_del(p);
            memset(fengmen, 0, sizeof(FENGMEN));
            list_add(p, &fengmenfreelist);
        }
    }
    //release welt
    WELT_PARAM *welt;
    list_for_each_safe(p, n, &co->welt) {
        welt = list_entry(p, WELT_PARAM, list);
        list_del(p);
        memset(welt, 0, sizeof(WELT_PARAM));
        list_add(p, &welt_param_freelist);
    }

    //release jacq and dis in co
    //MEM_ERR memerr;
    if (co->guid != NULL) {
        free(co->guid);
        co->guid = NULL;
        co->guidsize = 0;
    }

    if (co->jac != NULL) {
        free(co->jac);
        co->jac = NULL;
        co->jacsize = 0;
    }
    if (co->dis != NULL) {
        free(co->dis);
        co->dis = NULL;
        co->dissize = 0;
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
        //release btsr
        if (run->btsr != NULL) {
            freeBtsr(run);
        }
        stepJacqRelease(run);
        stepGuidRelease(run);
        memset(run, 0, sizeof*run);
    }
    memset(co, 0, sizeof*co);
}


static void sinkerparamToCo(CO_SINKERMOTORPARAM *coparam, SINKERMOTORPARAM *param) {
    for (int i = 0; i < 3; i++) {
        coparam->qf_feed[i] = param->qf_feed / MOTOR_SCALE;
        coparam->qi_feed[i] = param->qi_feed / MOTOR_SCALE;
    }
}

static void sizemotorparamToCo(CO_SIZEMOTORPARAM *coparam, SIZEMOTORPARAM *param) {
    for (int i = 0; i < 3; i++) {
        coparam->start[i] = param->start / MOTOR_SCALE;
        coparam->end[i] = param->end / MOTOR_SCALE;
    }
}


int32 coSave(S_CO *co, TCHAR *path) {
    //open co file
    int32 re = 0;
    uint32 br, wr;;
    struct list_head *p;
    FIL file;
    CO_HEADER *cohead;
    CO_SPEED *cospeed;
    CO_SIZEMOTOR_ZONE *cosizemotor;
    CO_SINKERMOTOR_ZONE *cosinkermotor;
    CO_ECONOMIZER_PARAM *co_eco;

    FRESULT r = f_open(&file, path, FA_WRITE | FA_READ);
    if (r != FR_OK) {
        re = CO_FILE_WRITE_ERROR;
        return re;
    }

    r = f_read(&file, cofilebuf, f_size(&file), &br);
    if (r != FR_OK && br != f_size(&file)) {
        re = CO_FILE_WRITE_ERROR;
        goto ERROR;
    }
    if (memcmp(cofilebuf, &co->head, sizeof co->head)) {
        re = CO_FILE_WRITE_DIFFILE;
        goto ERROR;
    }
    //speed
    cospeed = (CO_SPEED *)(cofilebuf + (co->head.sec[2].offset << 8) + co->cateinfo.speed_addr);
    list_for_each(p, &co->speed) {
        SPEED *speed = list_entry(p, SPEED, list);
        cospeed->step = speed->step;
        cospeed->rpm[0] = cospeed->rpm[1] = speed->rpm;
        for (int i = 0; i < 8; i++) {
            cospeed->ramp[i][0] = cospeed->ramp[i][1] = speed->ramp[i];
        }
        cospeed++;
    }
    //sizemotor
    cosizemotor = (CO_SIZEMOTOR_ZONE *)(cofilebuf + (co->head.sec[0].offset << 8) + co->resetinfo.sizemotorAddr);
    for (int i = 0; i < co->numofsizemotorzone; i++) {
        for (int j = 0; j < 8; j++) {
            sizemotorparamToCo(&cosizemotor[i].param[j], &co->sizemotor[i].param[j]);
        }
    }

    //sinkermotor
    cosinkermotor = (CO_SINKERMOTOR_ZONE *)(cofilebuf + (co->head.sec[0].offset << 8)
                                            + co->resetinfo.sinkermotor_feed1_3_Addr);
    for (int i = 0; i < co->numofsinkmoterzone_1_3; i++) {
        for (int j = 0; j < 8; j++) {
            sinkerparamToCo(&cosinkermotor[i].param[j], &co->sinkmoterzone_1_3[i].param[j]);
        }
    }

    //sinkermotor2-4
    cosinkermotor = (CO_SINKERMOTOR_ZONE *)(cofilebuf + (co->head.sec[0].offset << 8) + co->resetinfo.sinkermotor_feed2_4_Addr);
    for (int i = 0; i < co->numofsinkmoterzone_2_4; i++) {
        for (int j = 0; j < 8; j++) {
            sinkerparamToCo(&cosinkermotor[i].param[j], &co->sinkmoterzone_2_4[i].param[j]);
        }
    }
    //sinkermotor angle
    cosinkermotor = (CO_SINKERMOTOR_ZONE *)(cofilebuf + (co->head.sec[0].offset << 8) + co->resetinfo.sinkerangular_Addr);
    for (int i = 0; i < co->numofsinkangular; i++) {
        for (int j = 0; j < 8; j++) {
            sinkerparamToCo(&cosinkermotor[i].param[j], &co->sinkangular[i].param[j]);
        }
    }
    //economizer
    co_eco = (CO_ECONOMIZER_PARAM *)(cofilebuf + (co->head.sec[7].offset << 8) + co->supeinfo.econoAddrOffset);
    for (int i = 0; i < co->numofeconomizer; i++) {
        for (int j = 0; j < 8; j++) {
            co_eco[i].economize[j][0] = co->econo[i].economize[j];
            co_eco[i].economize[j][1] = co->econo[i].economize[j];
        }
    }

    //recalulate check data
    cohead = (CO_HEADER *)cofilebuf;
    cohead->sec[0].check = get_co_check(cofilebuf + (co->head.sec[0].offset << 8), co->head.sec[0].len << 8);
    cohead->sec[2].check = get_co_check(cofilebuf + (co->head.sec[2].offset << 8), co->head.sec[2].len << 8);
    cohead->sec[7].check = get_co_check(cofilebuf + (co->head.sec[7].offset << 8), co->head.sec[7].len << 8);
    cohead->coCheck = get_co_check(cofilebuf + 4, 512 - 4);

    //write to file
    f_lseek(&file, 0);
    r = f_write(&file, cofilebuf, f_size(&file), &wr);
    if (r != FR_OK && wr != f_size(&file)) {
        re = CO_FILE_WRITE_ERROR;
        goto ERROR;
    }
    f_close(&file);
    return re;

    ERROR:
    f_close(&file);
    return re;
}


static void cocreateindex_speed(S_CO_RUN *co_run, S_CO *co) {
    SPEED *speed;
    speed = list_first_entry_or_null(&co->speed, SPEED, list);
    if (speed == NULL || speed->step != 0) {
        speed = list_first_entry_or_null(&speedfreelist, SPEED, list);
        ASSERT(speed != NULL);
        speed->step = 0;
        speed->rpm = 50;
        speed->coed = false;
        for (int i = 0; i < 8; i++) {
            //speed->prerpm[i] = 0;
            speed->ramp[i] = 1;
        }
        list_move(&speed->list, &co->speed);
    }

    //init step->speed
    speed = list_first_entry(&co->speed, SPEED, list);
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (speed->step == i) {
            step->speed = speed;
            speed = list_entry(speed->list.next, SPEED, list);
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

static void cocreateindex_fengmen(S_CO_RUN *co_run, S_CO *co) {
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (list_empty(&co->fengmen[i])) {
            step->fengmen = NULL;
        } else {
            step->fengmen = &co->fengmen[i];
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
            for (int j = 0; j < 8; j++) {
                co_run->stepptr[i]->ilinetag[j] = co_run->stepptr[i]->istep;
            }
        }
        return;
    }
    for (int j = 0; j < 8; j++) {
        co_run->stepptr[0]->ilinetag[j] = 0;
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
            for (int i = beginstep->istep; i <= step->istep; i++) {
                co_run->stepptr[i]->econoFlag |= ECONO_INECONO;
            }
            unsigned econodif = co->econo[iecon].end - co->econo[iecon].begin + 1;
            for (int j = 0; j < 8; j++) {
                co_run->numofline[j] += econodif * (co->econo[iecon].economize[j] - 1) + 1;
                if (i != 0) { //set ECONO_END step ilinetag
                              //co_run->stepptr[i]->ilinetag[j] = co_run->stepptr[i - 1]->ilinetag[j] + 1;
                }
                if (i != co->numofstep - 1) { //set next step ilinetag
                    co_run->stepptr[i + 1]->ilinetag[j] = co_run->stepptr[i]->ilinetag[j]
                                                          + econodif * (co->econo[iecon].economize[j] - 1) + 1;
                }
            }
            iecon++;
        } else {
            step->econo = NULL;
            step->econoFlag = 0;
            for (int j = 0; j < 8; j++) {
                co_run->numofline[j]++;
                /*if (i != 0 && !IS_ECONO_END(*co_run->stepptr[i - 1])) {
                    co_run->stepptr[i]->ilinetag[j] = co_run->stepptr[i - 1]->ilinetag[j] + 1;
                }*/
                if (i != co->numofstep - 1) {
                    co_run->stepptr[i + 1]->ilinetag[j] = co_run->stepptr[i]->ilinetag[j] + 1;
                }
            }
        }
    }
}


static void cocreateindex_sinkmotor(S_CO_RUN *co_run, S_CO *co) {
    uint32 isinkmotorzone = 0;
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (co->sinkmoterzone_1_3[isinkmotorzone].head.beginStep <= i
            && co->sinkmoterzone_1_3[isinkmotorzone].head.endStep >= i) {
            step->sinkmoterzone_1_3 = &co->sinkmoterzone_1_3[isinkmotorzone];
            //step->sinkmoterzone_2_4 = &co->sinkmoterzone_2_4[isinkmotorzone];
            if (co->sinkmoterzone_1_3[isinkmotorzone].head.endStep == i) {
                isinkmotorzone++;
            }
        } else {
            step->sinkmoterzone_1_3 = NULL;
            // step->sinkmoterzone_2_4 = NULL;
        }
    }
    //calculate acc
    for (int i = 0; i < co->numofsinkmoterzone_1_3; i++) {
        SINKERMOTOR_ZONE *zone = &co->sinkmoterzone_1_3[i];
        uint32 beginstep = zone->head.beginStep;
        uint32 endstep = zone->head.endStep;
        for (int j = 0; j < 8; j++) {
            int32 linediff = co_run->stepptr[endstep]->ilinetag[j] - co_run->stepptr[beginstep]->ilinetag[j];
            if (linediff != 0) {
                zone->param[j].acc = (zone->param[j].qf_feed - zone->param[j].qi_feed) / linediff;
            } else {
                zone->param[j].acc = 0;
            }
        }
    }
    /*for (int i = 0; i < co->numofsinkmoterzone_2_4; i++) {
        SINKERMOTOR_ZONE *zone = &co->sinkmoterzone_2_4[i];
        uint32 beginstep = zone->head.beginStep;
        uint32 endstep = zone->head.endStep;
        for (int j = 0; j < 8; j++) {
            uint32 linediff = co_run->stepptr[endstep]->ilinetag[j] - co_run->stepptr[beginstep]->ilinetag[j];
            if (linediff != 0) {
                zone->param[j].acc = (zone->param[j].qf_feed - zone->param[j].qi_feed) / linediff;
            } else {
                zone->param[j].acc = 0;
            }
        }
    }*/
}


static void cocreateindex_sizemotor(S_CO_RUN *co_run, S_CO *co) {
    uint32 isizemotorzone = 0;
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (co->sizemotor[isizemotorzone].head.beginStep <= i
            && co->sizemotor[isizemotorzone].head.endStep >= i) {
            step->sizemotor = &co->sizemotor[isizemotorzone];
            if (co->sizemotor[isizemotorzone].head.endStep == i) {
                isizemotorzone++;
            }
        } else {
            step->sizemotor = NULL;
        }
    }
    for (int i = 0; i < co->numofsizemotorzone; i++) {
        SIZEMOTOR_ZONE *zone = &co->sizemotor[i];
        uint32 beginstep = zone->head.beginStep;
        uint32 endstep = zone->head.endStep;
        for (int j = 0; j < 8; j++) {
            int32 linediff = co_run->stepptr[endstep]->ilinetag[j] - co_run->stepptr[beginstep]->ilinetag[j];
            if (linediff != 0) {
                zone->param[j].acc = (zone->param[j].end - zone->param[j].start) / linediff;
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
        for (int i = param->weltinstep; i < param->weltoutstep; i++) {
            co_run->stepptr[i]->welt = true;
        }
    }
}



static void cocreateindex_jacq(S_CO_RUN *co_run, S_CO *co) {
    char *p = co->jac;
    JACKHEAD *head = (JACKHEAD *)p;
    p += head->addrBegin;
    unsigned int offset;

    co_run->seljacnum = 0;
    //parsed jacq
    MEM_ERR memerr;
    S_CO_RUN_STEP *step;
    for (int istep = 0; istep < co->numofstep; istep++){
        step = co_run->stepptr[istep];
        memset(step->jacsnum, 0, sizeof(step->jacsnum));
    }
    for (int istep = 0; istep < co->numofstep; istep++) {
        offset = *((unsigned int *)p + istep);
        CO_JACPAT *co_jac = (CO_JACPAT *)(offset + head->addrBegin + (int32)co->jac);
        step = co_run->stepptr[istep];
        for (; co_jac->isel != 0x80000000; co_jac++) {
            unsigned int isel = co_jac->isel;
            SEL_JAC *jac = MemGet(&jacmem, &memerr);
            ASSERT(memerr == MEM_ERR_NONE && jac != NULL);
            ASSERT(co_run->seljacnum < lenthof(co_run->seljac));
            co_run->seljac[co_run->seljacnum++] = jac;
            jac->disinfo = (DISINFO *)(((DISHEAD *)co->dis)->selInfoAddr
                                       + co_jac->disinfoAddr
                                       + (int32)co->dis);
            jac->co_jac = co_jac;
            jac->step = istep;
            for (int i = istep; i <= co_jac->outStep; i++) {
                step = co_run->stepptr[i];
                ASSERT(step->jacsnum[isel] < SEL_PRI_NUM);
                step->jacs[isel][step->jacsnum[isel]++] = jac;
                step->haveSel |= 0x01;
            }
        }
    }
}




static void cocreateindex_guid(S_CO_RUN *co_run, S_CO *co) {
    char *p = co->guid;
    GUIDHEAD *head = (GUIDHEAD *)p;
    p += head->addrBegin;
    unsigned int offset;

    co_run->selguidnum = 0;
    //parsed guid
    MEM_ERR memerr;
    for (int istep = 0; istep < co->numofstep; istep++) {
        offset = *((unsigned int *)p + istep);
        CO_GUID *co_guid = (CO_GUID *)(offset + head->addrBegin + (int32)co->guid);
        S_CO_RUN_STEP *step = co_run->stepptr[istep];
        for (; co_guid->ifeed != 0x80000000; co_guid++) {
            unsigned int ifeed = co_guid->ifeed;
            SEL_GUID *guid = MemGet(&guidmem, &memerr);
            ASSERT(memerr == MEM_ERR_NONE && guid != NULL);
            co_run->selguid[co_run->selguidnum++] = guid;
            guid->disaddr += ((DISHEAD *)co->dis)->guidInfoAddr
                             + co_guid->addr
                             + (int32)co->dis;
            guid->co_guid = co_guid;
            guid->step = istep;
            for (int i = istep; i <= co_guid->outStep; i++) {
                step = co_run->stepptr[i];
                if (ifeed / co->machine->feedNum) { //CAM
                    ifeed %= 4;
                    step->cam[ifeed] = guid;
                    step->haveSel |= 0x02;
                } else {
                    step->finger[ifeed] = guid;
                    step->haveSel |= 0x04;
                }
            }
        }
    }
}


static void btsrLoadFile(BTSR *btsr, bool *havematchbtsrfile) {
    *havematchbtsrfile = false;
    wchar_t btsrfilepath[200];
    wcscpy(btsrfilepath, FILEPATH);
    wcscat(btsrfilepath, btsr->name);
    fileFixNameReplace(btsrfilepath, L".btr");
    FIL btsrfile;
    BTSR btsrtmp;
    uint32 br, offset, datasize;
    if (f_open(&btsrfile, btsrfilepath, FA_READ | FA_OPEN_EXISTING) != FR_OK) {
        return;
    }

    if (FR_OK != f_read(&btsrfile, &btsrtmp, sizeof btsrtmp,&br) || br != sizeof btsrtmp) {
        goto FILE_ERROR;
    }
    offset = (uint32)btsrtmp.data;
    if (btsrtmp.coCheck != btsr->coCheck ||
        wcscmp(btsrtmp.name, btsr->name) != 0 ||
        btsrtmp.cosize != btsr->cosize ||
        btsrtmp.num != btsr->num ||
        btsrtmp.point != btsr->point ||
        btsrtmp.numofline != btsr->numofline) {
        goto FILE_ERROR;
    }
    if (FR_OK != f_lseek(&btsrfile, offset)) {
        goto FILE_ERROR;
    }
    datasize = btsrtmp.num * btsrtmp.point * btsrtmp.numofline;
    if (FR_OK != f_read(&btsrfile, btsr->data, datasize, &br) || br != datasize) {
        goto FILE_ERROR;
    }
    f_close(&btsrfile);
    btsr->dataAvailable = true;
    *havematchbtsrfile = true;
    return;
    FILE_ERROR:
    f_close(&btsrfile);
    f_unlink(btsrfilepath);
}


static BTSR* allocBtsr(S_CO_RUN *co_run, int32 num, int32 point, int cosize) {
    struct list_head *p;
    BTSR *btsr = NULL;
    ASSERT(co_run->co);
    ASSERT(cosize < 8);
    S_CO *co = co_run->co;
    uint32 cocheck = co->head.coCheck;
    const wchar_t *coname = co->filename;
    list_for_each(p, &btsrlist) {
        btsr = list_entry(p, BTSR, list);
        if (wcscmp(btsr->name, coname) == 0 && btsr->coCheck == cocheck &&
            btsr->num == num && btsr->point == point && btsr->cosize == cosize &&
            btsr->numofline == co_run->numofline[cosize]) { //find btsr in btsrlist
            btsr->ref++;
            return btsr;
        }
    }
    //if not find, get btsr frome btsr_freelist and add to btsrlist
    btsr = list_first_entry_or_null(&btsr_freelist, BTSR, list);
    ASSERT(btsr != NULL);
    btsr->ref = 1;
    btsr->coCheck = cocheck;
    btsr->dataAvailable = false;
    btsr->num = num;
    uint32 numofpoint = btsr->point = point;
    btsr->cosize = cosize;
    uint32 numofline = btsr->numofline = co_run->numofline[cosize];
    uint32 btsrdatasize = num * numofpoint * numofline;
    MEM_ERR memerr;
    btsr->data = (char *)MemGet(&btsr_dis_mem, &memerr);
    ASSERT(btsr->data != NULL && memerr == MEM_ERR_NONE && btsrdatasize <= BTSR_DIS_BUF_SIZE);
    btsr->datapointer = 0;
    ASSERT(wcslen(co->filename) < 50);
    wcscpy(btsr->name, co->filename);
    list_move(&btsr->list, &btsrlist);
    bool dumy;
    btsrLoadFile(btsr, &dumy);
    co_run->btsr = btsr;
    return btsr;
}


static void freeBtsr(S_CO_RUN *co_run) {
    ASSERT(co_run->btsr);
    BTSR *btsr = co_run->btsr;
    if (--btsr->ref == 0) { //only 1 ref,  release btsr
        if (btsr->data) {
            MEM_ERR memerr;
            MemPut(&btsr_dis_mem, btsr->data, &memerr);
            ASSERT(memerr == MEM_ERR_NONE);
            btsr->data = NULL;
        }
        list_move(&btsr->list, &btsr_freelist);
    }
    co_run->btsr = NULL;
}


void coRunInitBtsr(S_CO_RUN *co_run, int numofbtsr, int numofpoint, int co_size) {
    if (co_run->btsr) {
        freeBtsr(co_run);
    }
    allocBtsr(co_run, numofbtsr, numofpoint, co_size);
}


void coRunBtsrBeginStudy(S_CO_RUN *co_run) {
    if (co_run->btsr) {
        co_run->btsr->dataAvailable = false;
        co_run->btsr->datapointer = 0;
    }
}

void coRunBtsrStudy(S_CO_RUN *co_run, void *buf, int size) {
    BTSR *btsr = co_run->btsr;
    int32 sizeofLineData = btsr->num * btsr->point;
    ASSERT(sizeofLineData == size);
    ASSERT(co_run->btsr && co_run->btsr->data);
    uint32 offset = co_run->btsr->datapointer;
    int32 sizeofdata = btsr->numofline * sizeofLineData;
    ASSERT(offset + size <= sizeofdata);
    if (offset + size == sizeofdata) {
        btsr->dataAvailable = true;
    }
    memcpy(co_run->btsr->data + offset, buf, size);
    btsr->datapointer += size;
}



bool coRunIsBtsrDataAvailable(S_CO_RUN *co_run) {
    if (!co_run->btsr) {
        return false;
    }
    return(bool)co_run->btsr->dataAvailable;
}


bool coRunBtsrData(S_CO_RUN *co_run, int32 iline, void **data, uint32 *datasize) {
    if (co_run->btsr && co_run->btsr->data) {
        BTSR *btsr = co_run->btsr;
        uint32 sizeofline = iline * btsr->point * btsr->num;
        ASSERT(iline < btsr->numofline);
        *data = &btsr->data[sizeofline * iline];
        *datasize = sizeofline;
        return true;
    }
    *datasize = 0;
    *data = NULL;
    return false;
}


BOOL coRunBtsrSave(S_CO_RUN *co_run) {
    if (co_run->btsr == NULL) {
        return false;
    }
    if (!co_run->btsr->dataAvailable) {
        return false;
    }
    S_CO *co = co_run->co;
    wchar_t btsrfilepath[200];
    wcscpy(btsrfilepath, FILEPATH);
    wcscat(btsrfilepath, co->filename);
    fileFixNameReplace(btsrfilepath, L".btr");
    FIL btsrfile;
    if (f_open(&btsrfile, btsrfilepath, FA_CREATE_ALWAYS | FA_READ | FA_WRITE) != FR_OK) {
        return false;
    }
    uint32 wr;
    int32 sizeofdata;
    BTSR btemp;
    btemp = *co_run->btsr;
    btemp.data = (char *)256;
    btemp.datapointer = 0;

    if (FR_OK != f_write(&btsrfile, &btemp, sizeof btemp,&wr) || wr != sizeof btemp) {
        goto ERROR;
    }
    f_lseek(&btsrfile, 256);
    sizeofdata = co_run->btsr->numofline * co_run->btsr->point * co_run->btsr->num;
    if (FR_OK != f_write(&btsrfile, co_run->btsr->data, sizeofdata, &wr) || wr != sizeofdata) {
        goto ERROR;
    }
    f_close(&btsrfile);

#if BTSR_DEBUG == 1
    wchar_t despath[100];
    wcscpy(despath, btsrfilepath);
    despath[0] = L'2';
    char cpbuf[1024];
    f_copy(btsrfilepath, despath, cpbuf, sizeof cpbuf);
#endif

    return true;
    ERROR:
    f_close(&btsrfile);
    f_unlink(btsrfilepath);
    return false;
}




void coCreateIndex(S_CO_RUN *co_run, S_CO *co) {
    //uint32 flag = 0;
    if (!co->parsed) {
        return;
    }
    INIT_LIST_HEAD(&co_run->step);

    co_run->btsr = NULL;
    //create index and list
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = list_entry(runstepfreelist.next, S_CO_RUN_STEP, list);
        co_run->stepptr[i] = step;
        step->istep = i;
        list_move_tail(&step->list, &co_run->step);
    }

    //economizer
    cocreateindex_econ(co_run, co);
    //speed;
    cocreateindex_speed(co_run, co);
    //welt;
    cocreateindex_welt(co_run, co);


    //sizemotor;
    cocreateindex_sizemotor(co_run, co);
    //sinkmotor
    cocreateindex_sinkmotor(co_run, co);
    //func
    cocreateindex_func(co_run, co);
    //fengmen
    cocreateindex_fengmen(co_run, co);
    //dis
    cocreateindex_jacq(co_run, co);
    //guid
    cocreateindex_guid(co_run, co);

    co_run->numofstep = co->numofstep;
    co->run = co_run;
    co_run->co = co;
}




static void funcodeParse(struct list_head *func, ACT_GROUP *angleValve, ALARM_GROUP *angleAlarm, uint32 *flag);

static int32 mathCalcuLineFunc(int32 y1, int32 y2, int32 x1, int32 x, int32 acc) {
    int32 val = y1 + acc * (x - x1);
    if ((acc > 0 && val > y2) || (acc < 0 && val < y2)) {
        val = y2;
    }
    return val;
}


static void corunCalcSpeed(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size) {
    //calculate speed;
    int16 acc;
    S_CO_RUN_STEP *step = co_run->stepptr[line->istep];
    if (step->speed != NULL) {
        int32 tarrpm = step->speed->rpm;
        int32 ramp = step->speed->ramp[size];
        acc = (tarrpm - (int32)line->prerpm) / ramp;
        line->speedAcc = acc;
        line->targetSpeed = tarrpm;
    }

    line->rpm = line->prerpm + line->speedAcc;
    if (line->speedAcc < 0 && line->rpm < line->targetSpeed) {
        line->rpm = line->targetSpeed;
    } else if (line->speedAcc > 0 && line->rpm > line->targetSpeed) {
        line->rpm = line->targetSpeed;
    }
    if (line->rpm == line->targetSpeed) {
        line->speedAcc = 0;
    }
}


static void corunCalcSizemotor(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size) {
    S_CO_RUN_STEP *step = co_run->stepptr[line->istep];

    SIZEMOTOR_ZONE *sizemotor_zone = step->sizemotor;
    if (sizemotor_zone != NULL) {
        line->zonename = step->sizemotor->descrpition;
        line->zonebegin = step->sizemotor->head.beginStep;
        line->zoneend = step->sizemotor->head.endStep;
        uint32 sizemotorbase = step->sizemotor->param[size].start;
        int32 sizemotoracc = step->sizemotor->param[size].acc;
        uint32 sizemotorend = step->sizemotor->param[size].end;
        uint32 sizemotorbaseline = co_run->stepptr[step->sizemotor->head.beginStep]->ilinetag[size];

        line->sizemotor = mathCalcuLineFunc(sizemotorbase, sizemotorend,
                                            sizemotorbaseline, line->iline,
                                            sizemotoracc);
        /*if (IS_ECONO_BEGIN_END(*step)) {
            if (line->iline == step->ilinetag[size]) {
                line->stepSizemotorBase = mathCalcuLineFunc(sizemotorbase, sizemotorend,
                                                            sizemotorbaseline, step->ilinetag[size],
                                                            sizemotoracc);
                line->stepSizemotorAcc = sizemotoracc;
            }
        } else {
            line->stepSizemotorBase = 0;
            line->stepSizemotorAcc = 0;
        }*/
    } else {
        line->zonename = NULL;
        line->zonebegin = 0;
        line->zoneend = 0;
    }
}


static void corunCalcSinkermotor1_3(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size) {
    S_CO_RUN_STEP *step = co_run->stepptr[line->istep];
    SINKERMOTOR_ZONE *sinkermotor_zone_1_3 = step->sinkmoterzone_1_3;
    if (sinkermotor_zone_1_3 != NULL) {
        uint32 sinkermotorbase = sinkermotor_zone_1_3->param[size].qi_feed;
        int32 sinkermotoracc = sinkermotor_zone_1_3->param[size].acc;
        uint32 sinkermotorend = sinkermotor_zone_1_3->param[size].qf_feed;
        uint32 sinkermotorbaseline = co_run->stepptr[sinkermotor_zone_1_3->head.beginStep]->ilinetag[size];
        line->sinkmotor1_3 = mathCalcuLineFunc(sinkermotorbase, sinkermotorend,
                                               sinkermotorbaseline, line->iline,
                                               sinkermotoracc);
        /*if (IS_ECONO_BEGIN_END(*step)) {
            if (line->iline == step->ilinetag[size]) {
                line->stepSinkermotor1_3Base = mathCalcuLineFunc(sinkermotorbase, sinkermotorend,
                                                                 sinkermotorbaseline, step->ilinetag[size],
                                                                 sinkermotoracc);
                line->stepsinkermotor1_3Acc = sinkermotoracc;
            } else {
                line->stepSinkermotor1_3Base = 0;
                line->stepsinkermotor1_3Acc = 0;
            }
        }*/
    }
}


typedef struct {
    uint32 linenum;
    uint32 addr;
}DIS_GUID_ENTRY;

typedef struct {
    //uint16 niddleNum;
    uint16 niddle;
    uint8 act;
    uint8 state;
}DIS_GUID_ACT;



static uint16 fingercamcode[FEED_NUM][10] = {
    { Gu1_1, Gu2_1, Gu3_1, Gu4_1, Gu5_1, Gu6_1, Gu1v1, Gu3v1, GURF1, GURA1},
    { Gu1_2, Gu2_2, Gu3_2, Gu4_2, Gu5_2, Gu6_2, Gu1v2, Gu3v2, GURF2, GURA2},
    { Gu1_3, Gu2_3, Gu3_3, Gu4_3, Gu5_3, Gu6_3, Gu1v3, Gu3v3, GURF3, GURA3},
    { Gu1_4, Gu2_4, Gu3_4, Gu4_4, Gu5_4, Gu6_4, Gu1v4, Gu3v4, GURF4, GURA4}
};


uint32 corunReadDisfingerCam(S_CO_RUN *co_run, S_CO_RUN_LINE *run_line, unsigned int niddle, unsigned int cosize, uint16 valveCode[]) {
    uint32 codenum = 0;
    uint32 machineNiddleNum = machine.niddleNum;
    //niddle += (niddle + machine.selPreNiddleNum) / machineNiddleNum;
    uint32 line = run_line->iline;
    S_CO_RUN_STEP *step = co_run->stepptr[run_line->istep];
    for (int i = 0; i < FEED_NUM; i++) {
        SEL_GUID *guid = step->finger[i];
        if (guid == NULL) {
            continue;
        }
        uint32 guidbaseline = co_run->stepptr[guid->step]->ilinetag[cosize];
        uint32 outstep_p1 = guid->co_guid->outStep + 1;
        uint32 guidendline;
        if (outstep_p1 != co_run->numofstep) {
            guidendline = co_run->stepptr[outstep_p1]->ilinetag[cosize] - 1;
        } else {
            guidendline = co_run->numofline[cosize] - 1;
        }
        int32 niddlebegin = guid->co_guid->inNiddle;
        int32 niddleoff = machineNiddleNum * (line - guidbaseline) + niddle - niddlebegin;
        int32 niddleoffmax = machineNiddleNum * (guidendline - guidbaseline) + guid->co_guid->outNiddle - guid->co_guid->inNiddle;
        if (niddleoff >= 0 && niddleoff <= niddleoffmax) {
            DIS_GUID_ENTRY *dis_entry = (DIS_GUID_ENTRY *)guid->disaddr;
            uint32 guidloopniddle = dis_entry->linenum * machineNiddleNum; //diddle num per jac loop
            uint32 guidniddleinloop = niddleoff % guidloopniddle;  //niddle in loop
            uint32 guidline = guidniddleinloop / machineNiddleNum; // guid line
            uint32 iguid = guidniddleinloop % machineNiddleNum;
            //uint32 jacrange = jac->disinfo->endNiddle - jac->disinfo->beginNiddle;
            //if (ijac > jacrange) {
            //    continue;
            //}
            uint32 *feedactaddr =(uint32 *)((uint32)co_run->co->dis
                                +  dis_entry->addr
                                + ((DISHEAD *)(co_run->co->dis))->guidInfoAddr);

            DIS_GUID_ACT *act = (DIS_GUID_ACT *)(feedactaddr[guidline]+((DISHEAD *)(co_run->co->dis))->guidDataAddr+(uint32)co_run->co->dis+2);

            for (int j = 0; act[j].niddle != 0xffff; j++) {
                if (act[j].niddle == iguid) {
                    valveCode[codenum] = fingercamcode[i][act[j].act];
                    valveCode[codenum] |= !!act[j].state << 12;
                    codenum++;
                }
            }
        }
    }
    return codenum;
}


int32 corunReadLine(S_CO_RUN *co_run, S_CO_RUN_LINE *line, const S_CO_RUN_LINE *preline, uint32 size) {
    ASSERT(preline->co_run == co_run);
    line->co_run = preline->co_run;
    int32 preistep = preline->istep;
    int32 preiline = preline->iline;
    if (preiline >= co_run->numofline[size] - 1) { // preline is last line
        return -1;
    }
    S_CO_RUN_STEP *prestep = co_run->stepptr[preistep];

    if (IS_ECONO_END(*prestep)) {
        if (preline->iecono == prestep->econo->economize[size] - 1) { // last loop
            line->iecono = 0;
            line->istep = preistep + 1;
            line->iline = preiline + 1;
        } else { // not last loop
            line->iecono = preline->iecono + 1;
            line->istep = preline->econobegin;
            line->iline = prestep->ilinetag[size];
            line->iline = prestep->ilinetag[size] +
                          (prestep->econo->end - prestep->econo->begin + 1) * line->iecono;
        }
    } else {
        line->istep = preistep + 1;
        line->iline = preiline + 1;
        line->iecono = preline->iecono;
    }
    int32 istep = line->istep;
    S_CO_RUN_STEP *step = co_run->stepptr[istep];

    if (IS_ECONO_INECONO(*step)) {
        line->econobegin = step->econo->begin;
        line->econoend = step->econo->end;
        line->econonum = step->econo->economize[size];
    } else {
        line->econobegin = istep;
        line->econoend = istep;
        line->econonum = 1;
    }

//calculate speed
    line->prerpm = preline->rpm;
    line->speedAcc = preline->speedAcc;
    line->targetSpeed = preline->targetSpeed;
    corunCalcSpeed(co_run, line, size);

//calculate sizemotor;
//line->stepSizemotorBase = preline->stepSizemotorBase;
//line->stepSizemotorAcc = preline->stepSizemotorAcc;
    corunCalcSizemotor(co_run, line, size);

//calculate sinker motor_1_3
//line->stepSinkermotor1_3Base = preline->stepSinkermotor1_3Base;
//line->stepsinkermotor1_3Acc = preline->stepsinkermotor1_3Acc;
    corunCalcSinkermotor1_3(co_run, line, size);

//calculate sinker motor_2_4
/////////////////////////////////////////
//dis-finger dis-cam
//corunReadDisfingerCam(co_run,line,size);
//welt;
    line->welt = step->welt;

//funcode
    line->flag = 0;
    funcodeParse(step->func, line->act, line->alarm, &line->flag);
//guid


//fengmen
    if (step->fengmen != NULL && istep != preistep) {
        line->isfengmenAct = true;
        memset(line->fengmen, 0, sizeof line->fengmen);
        struct list_head *p;
        FENGMEN *fengmenp;
        list_for_each(p, step->fengmen) {
            fengmenp = list_entry(p, FENGMEN, list);
            line->fengmen[fengmenp->angular] = fengmenp;
        }
    } else {
        line->isfengmenAct = false;
    }

    //jacq guid flag
    line->flag |= step->haveSel<<8;

    return co_run->numofline[size] - line->iline - 1;
}



uint16 coRunReadJacq(S_CO_RUN *co_run, S_CO_RUN_LINE *run_line, unsigned int niddle, unsigned int cosize) {
    uint8 seldata = 0, selmask = 0;
    uint8 slzseldata = 0, slzselmask = 0;
    uint8 patseldata = 0, patselmask = 0;
    uint8 patsupand = 0xff, patsupor = 0;
    ASSERT(niddle < machine.niddleNum);
    uint32 machineNiddleNum = machine.niddleNum;
    niddle += (niddle + machine.selPreNiddleNum) / machineNiddleNum;
    uint32 line = run_line->iline;
    S_CO_RUN_STEP *step = co_run->stepptr[run_line->istep];
    if ((niddle + machine.selPreNiddleNum) / machineNiddleNum > 0) { // sel pre compensate
        line++;
        if ((step->istep + 1) != co_run->numofstep && line == (step + 1)->ilinetag[cosize]) {
            step++;
        }
    }
    for (int i = 0; i < FEED_NUM; i++) {
        for (int j = 0; j < step->jacsnum[i]; j++) {
            SEL_JAC *jac = step->jacs[i][j];
            uint32 jacbaseline = co_run->stepptr[jac->step]->ilinetag[cosize];
            uint32 outstep_p1 = jac->co_jac->outStep + 1;
            uint32 jacendline;
            if (outstep_p1 != co_run->numofstep) {
                jacendline = co_run->stepptr[outstep_p1]->ilinetag[cosize] - 1;
            } else {
                jacendline = co_run->numofline[cosize] - 1;
            }
            int32 niddlebegin = jac->co_jac->inNiddle;
            int32 niddleoff = machineNiddleNum * (line - jacbaseline) + niddle - niddlebegin;
            int32 niddleoffmax = machineNiddleNum * (jacendline - jacbaseline) + jac->co_jac->outNiddle - jac->co_jac->inNiddle;
            if (niddleoff >= 0 && niddleoff <= niddleoffmax) {
                uint32 jacloopniddle = jac->disinfo->num * machineNiddleNum; //diddle num per jac loop
                uint32 jacniddleinloop = niddleoff % jacloopniddle;  //diddle in loop
                uint32 jacline = jacniddleinloop / machineNiddleNum; // jacq line
                uint32 ijac = jacniddleinloop % machineNiddleNum;
                uint32 jacrange = jac->disinfo->endNiddle - jac->disinfo->beginNiddle;
                if (ijac > jacrange) {
                    continue;
                }
                uint32 ijacdword = ijac / 32;
                uint32 ijacbitmask = 1 << (31 - ijac % 32);
                uint32 jackcodataaddr = *(&jac->disinfo->num + jacline + 1) +
                                        (uint32)co_run->co->dis +
                                        ((DISHEAD *)(co_run->co->dis))->selDataAddr;

                uint32 jackcodata = ((CO_DIS_DATA *)jackcodataaddr)->data[ijacdword];

                switch (jac->co_jac->selType) {
                case JACQTYPE_SLZ:
                    slzseldata |= !!(jackcodata & ijacbitmask) << i;
                    slzselmask |= (1 << i);
                    break;
                case JACQTYPE_PAT:
                    patseldata |= !!(jackcodata & ijacbitmask) << i;
                    patselmask |= (1 << i);
                    break;
                case JACQTYPE_SUP0:
                    patsupand &= ~(!!(~jackcodata & ijacbitmask) << i);
                    break;
                case JACQTYPE_SUP1:
                    patsupor |= !!(jackcodata & ijacbitmask) << i;
                    break;
                default:
                    break;
                }
            }
        }
    }
    selmask = slzselmask | patselmask;
    patseldata &= patsupand;
    patseldata |= patsupor;
    seldata = (patseldata & ~slzselmask) | (slzseldata & slzselmask);
    return(selmask | !!selmask << 7) << 8 | seldata;
}



#if 0
uint16 coRunReadJacq(S_CO_RUN *co_run, S_CO_RUN_LINE *run_line, unsigned int niddle, unsigned int cosize) {
    uint8 seldata = 0,selmask = 0;
    uint8 slzseldata = 0,slzselmask = 0;
    uint8 patseldata = 0,patselmask = 0;
    uint8 patsupand = 0xff , patsupor = 0;
    for (int i = 0; i < FEED_NUM; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[run_line->istep];
        uint32 line = run_line->iline;
        for (int j=0;j<step->jacsnum[i];j++) {
            SEL_JAC *jac = step->jacs[i][j];
            uint32 jacbaseline = co_run->stepptr[jac->step]->ilinetag[cosize];
            uint32 outstep_p1 = jac->co_jac->outStep + 1;
            uint32 jacendline;
            if (outstep_p1 != co_run->numofstep) {
                jacendline = co_run->stepptr[outstep_p1]->ilinetag[cosize] - 1;
            } else {
                jacendline = co_run->numofline[cosize] - 1;
            }
            int32 niddlebegin = jac->co_jac->inNiddle;
            int32 niddleoff = machineNiddleNum * (line - jacbaseline) + niddle - niddlebegin;
            int32 niddleoffmax = machineNiddleNum * (jacendline - jacbaseline) + jac->co_jac->outNiddle - jac->co_jac->inNiddle;
            if (niddleoff >= 0 && niddleoff <= niddleoffmax) {
                uint32 jacloopniddle = jac->disinfo->num * machineNiddleNum; //diddle num per jac loop
                uint32 jacniddleinloop = niddleoff % jacloopniddle;  //diddle in loop
                uint32 jacline = jacniddleinloop / machineNiddleNum; // jacq line
                uint32 ijac = jacniddleinloop % machineNiddleNum;
                uint32 jacrange = jac->disinfo->endNiddle - jac->disinfo->beginNiddle;
                if (ijac > jacrange) {
                    continue;
                }
                uint32 ijacdword = ijac / 32;
                uint32 ijacbitmask = 1 << (31 - ijac % 32);
                uint32 jackcodataaddr = *(&jac->disinfo->num + jacline+1) +
                                        (uint32)co_run->co->dis +
                                        ((DISHEAD *)(co_run->co->dis))->selDataAddr;

                uint32 jackcodata = ((CO_DIS_DATA *)jackcodataaddr) -> data[ijacdword];

                switch (jac->co_jac->selType) {
                case JACQTYPE_SLZ:
                    slzseldata |= !!(jackcodata & ijacbitmask) << i;
                    slzselmask |= (1 << i);
                    break;
                case JACQTYPE_PAT:
                    patseldata |= !!(jackcodata & ijacbitmask) << i;
                    patselmask |= (1 << i);
                    break;
                case JACQTYPE_SUP0:
                    //patsupand  &=  !!(jackcodata & ijacbitmask)<<i;
                    break;
                case JACQTYPE_SUP1:
                    //patsupor  |= !!(jackcodata & ijacbitmask)<<i;
                    break;
                default:
                    break;
                }
            }
        }
    }
    selmask = slzselmask | patselmask;
    //patseldata &= patsupand;
    //patseldata |= patsupor;
    seldata = (patseldata & ~slzselmask) | (slzseldata & slzselmask);
    return(selmask | !!selmask<<7)<<8 | seldata;
}

#endif


void corunRollStep(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size) {
    ASSERT(line->co_run == co_run);
    //line->iline = line->nextline;
    S_CO_RUN_STEP *step = co_run->stepptr[line->istep];
    //S_CO_RUN_STEP *nextstep = co_run->stepptr[line->istep + 1];

    //process step ,calculate next step
    if (IS_ECONO_BEGIN_END(*step)) {
        if (line->iecono < line->econonum - 1) { //not last loop
            line->iline++;
            line->iecono++;
        } else { //last loop
            line->iline = step->ilinetag[size];
            line->iecono = 0;
        }
    }

    //calculate speed;
    line->rpm = 200;

    //calculate sizemotor;
    corunCalcSizemotor(co_run, line, size);

    //calculate sinker motor_1_3
    corunCalcSinkermotor1_3(co_run, line, size);
}




uint32 corunReadStep(S_CO_RUN *co_run, S_CO_RUN_LINE *line, const S_CO_RUN_LINE *preline, uint32 size) {
    ASSERT(preline->co_run == co_run);
    line->co_run = preline->co_run;
    int32 preistep = preline->istep;

    if (preistep >= co_run->numofstep - 1) {
        return -1;
    }

    //process step
    int32 istep = line->istep = preline->istep + 1;

    line->iline = co_run->stepptr[istep]->ilinetag[size];

    S_CO_RUN_STEP *step = co_run->stepptr[istep];

    line->iecono = 0;
    if (IS_ECONO_INECONO(*step)) {
        line->econobegin = step->econo->begin;
        line->econoend = step->econo->end;
        line->econonum = step->econo->economize[size];
    } else {
        line->econobegin = istep;
        line->econoend = istep;
        line->econonum = 1;
    }

    //calculate speed;
    line->prerpm = preline->rpm;
    line->speedAcc = preline->speedAcc;
    line->targetSpeed = preline->targetSpeed;
    corunCalcSpeed(co_run, line, size);
    line->rpm = MIN(200, line->rpm);

    //calculate sizemotor
    line->stepSizemotorBase = preline->stepSizemotorBase;
    line->stepSizemotorAcc = preline->stepSizemotorAcc;
    corunCalcSizemotor(co_run, line, size);

    //calculate sinker motor_1_3
    line->stepSinkermotor1_3Base = preline->stepSinkermotor1_3Base;
    line->stepsinkermotor1_3Acc = preline->stepsinkermotor1_3Acc;
    corunCalcSinkermotor1_3(co_run, line, size);
    //corunCalcSinkermotor1_3(co_run, line, size);

    //welt;
    line->welt = step->welt;

    //funcode
    funcodeParse(step->func, line->act, line->alarm, &line->flag);

    return co_run->numofstep - line->istep - 1;
}



void corunReset(S_CO_RUN *co_run, S_CO_RUN_LINE *line) {
    //seek 0
    line->co_run = co_run;
    line->istep = -1;
    line->iline = -1;
    line->prerpm = 0;
    line->rpm = 0;
    line->iecono = 0;                  //current economizer counter
    line->econonum = 1;                //economizer
    line->econobegin = 0;           //economizer begin step
    line->econoend = 0;             //economizer end step(end>begin)
    line->speedAcc = 0;                //speed acceleration when run
    line->targetSpeed = 0;             //target speed when ramp
    line->sizemotor = 0;
    line->sinkmotor1_3 = 0;
    line->sinkmotor2_4 = 0;
}



static const unsigned char cnhexData[272] = {
    0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x63, 0x6E, 0x20, 0x4c,
    0x35, 0x31, 0x30, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x02, 0x05, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x43, 0x4F, 0x4E, 0x43, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD1, 0x8D, 0x72, 0x14,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x4E, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4E, 0x00, 0x41
};


static void cocnfilenameformat(char *namestr, char buf[8]) {
    //set filename to cn buf
    memset(buf, 0x20, 8);
    buf[0] = 0;
    for (int i = 0; i < 8; i++) {
        if (namestr[i] != 0 && namestr[i] != '.') {
            buf[i] = namestr[i];
        } else {
            break;
        }
    }
}




static void cnGroupTofile(S_CN_GROUP *group, CN_GROUP *group_file) {
    cocnfilenameformat(group->filename, group_file->co);
    *(uint32 *)(&group_file->co[8]) = 0x206f632e; //".co"
    group_file->unknow = 0x4e01;
    group_file->product = group->num;
    const static char cnco_unknown[12] = { 0x4E, 0x00, 0x00, 0x00, 0x00, 0x41,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    memcpy(group_file->dummy, cnco_unknown, sizeof cnco_unknown);
}


bool cnCreate(const TCHAR *path, S_CN_GROUP *co, uint32 num) {
    FIL file;
    FILINFO fileinfo;
    uint32 bw;
    FRESULT r;
    uint8 buf[512];
    char filename[13];
    memset(buf, 0, sizeof buf);
    memcpy(buf, cnhexData, sizeof cnhexData);
    r = f_open(&file, path, FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
    if (r != FR_OK) {
        return false;
    }
    f_sync(&file);
    fileinfo.lfname = NULL;
    f_stat(path, &fileinfo);

    //set filename to cn buf
    for (int i = 0; i < 13; i++) {
        filename[i] = fileinfo.fname[i];
    }
    strtok(filename, ".");
    for (int i = 0; i < 8; i++) {
        if (filename[i] != 0) {
            buf[4 + i] = filename[i];
        } else {
            break;
        }
    }
    //set 5 CN_GROUP
    if (num > 5) {
        num = 5;
    }
    for (int i = 0; i < num; i++) {
        CN_GROUP group_file;
        cnGroupTofile(co + i, &group_file);
        memcpy(&buf[0x16a + 30 * i], &group_file, sizeof group_file);
    }
    *(uint32 *)buf = get_co_check(buf + 4, 256 - 4);

    r = f_write(&file, buf, 512, &bw);
    if (r != FR_OK || bw != 512) {
        f_close(&file);
        f_unlink(path);
        return false;
    }
    f_close(&file);
    return true;
}



int cnParse(const TCHAR *path, S_CN_GROUP *val) {
    //open co file
    FIL file;
    uint32 br, check;
    int32 re = CN_OK;
    char buf[256];
    CN_GROUP cogp;
    FRESULT r = f_open(&file, path, FA_READ);
    if (r != FR_OK) {
        re = CN_READ_ERROR;
        return re;
    }
    if (f_size(&file) != 512) {
        re = CN_FILE_ERROR;
        goto ERROR;
    }
    r = f_read(&file, buf, sizeof buf,&br);
    if (r != FR_OK && br != sizeof buf) {
        re = CN_READ_ERROR;
        goto ERROR;
    }
    check = get_co_check(buf + 4, 256 - 4);
    if (check != *(uint32 *)buf) {
        re = CN_FILE_ERROR;
        goto ERROR;
    }
    f_lseek(&file, 0x16a);

    for (int i = 0; i < 5; i++) {
        r = f_read(&file, &cogp, sizeof cogp,&br);
        if (r != FR_OK && br != sizeof cogp) {
            re = CN_READ_ERROR;
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
    return re;
    ERROR:
    f_close(&file);
    return re;
}


static void funcodeResolve(FUNC *fun, uint16 *valvecode, uint32 *valnum,
                           uint16 *alarmcode, uint32 *alarmnum, uint32 *flag);

static void funcodeParse(struct list_head *func, ACT_GROUP *angleValve, ALARM_GROUP *angleAlarm, uint32 *flag) {
    struct list_head *p;
    for (int i = 0; i < 360; i++) {
        angleValve[i].num = 0; //cear ACT_GROUP::num
        angleAlarm[i].num = 0; //cear ALARM_GROUP::num
    }

    if (func == NULL) {
        return;
    }
    list_for_each(p, func) {
        FUNC *fun = list_entry(p, FUNC, list);
        uint32 angle = fun->angular;
        //uint32 angle = Angle_To_Needles(fun->angular);  //change angle to needle
        uint32 valvecodenum = 0, alarmnum = 0, flagtemp = 0;
        funcodeResolve(fun, &angleValve[angle].valvecode[angleValve[angle].num], &valvecodenum,
                       &angleAlarm[angle].alarmcode[angleAlarm[angle].num], &alarmnum,
                       &flagtemp);
        angleValve[angle].num += valvecodenum;
        angleAlarm[angle].num += alarmnum;
        *flag |= flagtemp;
    }
}


static uint16 L10L12_freeselcode2Valvecode(uint16 funcval) {
    uint16 valvecode;
    uint32 line, feed;
    if (funcval >= 0x65 && funcval <= 0xa4) { //free sel line 1 to line8
        valvecode = !((funcval - 0x65) % 2) << 12;  //in:1<<12  out:0<<12
        line = (funcval - 0x65) / 8;
        feed = (funcval - 0x65) / 2 % 4;
        valvecode |= SEL_LINE_NUMBER * feed + line + SEL_BASE;
    } else if (funcval >= 0xc5 && funcval <= 0x104) { //free sel line 9 to line 16
        valvecode = !((funcval - 0xc5) % 2) << 12;  //in:1<<12  out:0<<12
        line = (funcval - 0xc5) / 8 + 8;
        feed = (funcval - 0xc5) / 2 % 4;
        valvecode |= SEL_LINE_NUMBER * feed + line + SEL_BASE;
    }
    return valvecode;
}




static void L10L12_fixselcode2Valvecode(uint16 funcval, uint16 *valvecode, uint32 *num) {
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

static uint16 L10L12_hafuzhencode2Valvecode(uint16 funcval) {
    funcval -= 0x0d;
    uint32 valvecode = !(funcval & 0x01ul) << 12;
    valvecode |= HAFUZHEN_BASE + (funcval >> 1);
    return valvecode;
}

static uint16 L04E7_hafuzhencode2Valvecode(uint16 funcval) {
    funcval -= 0x02;
    uint32 valvecode = !(funcval & 0x01ul) << 12;
    valvecode |= HAFUZHEN_BASE + (funcval >> 1);
    return valvecode;
}


static uint16 common0506func2Valvecode(uint16 funcval) {
    return COMM_FUNC_BASE + funcval | 1 << 12;
}



static void camcode2Valvecode(FUNC *fun, uint16 *valvecode, uint32 *num) {
    static uint16 caminoutmap[4][3][3] = { //[feed][sxt][ace]
        { { 0, 293, 113 }, { 0, 293, 113}, { 0, 293, 113}},
        { { 0, 23, 203}, { 0, 23, 203}, { 0, 23, 203}},
        { { 0, 113, 293}, { 0, 113, 293}, { 0, 113, 293}},
        { { 0, 203, 23}, { 0, 203, 23}, { 0, 203, 23}},
    };

    bool in = false;
    uint32 pos_ace = fun->value % 3;
    uint32 sxt = fun->value / 3 % 3;
    uint32 feed = fun->value / 9;

    *num = 0;
    /*if (caminoutmap[feed][sxt][pos_ace] == 0xffff) {
        //TODO guide
        return;
    }*/
    if (caminoutmap[feed][sxt][pos_ace] == fun->angular) {
        in = true;
    }
    switch (pos_ace) {
    case 0: //A
        if (in == false) {
            valvecode[0] = CAM_BASE + feed * CAM_LINE_NUMBER + sxt * 2 + 1; //quan out
            valvecode[1] = CAM_BASE + feed * CAM_LINE_NUMBER + sxt * 2 + 0; //ban out
            *num = 2;
        } else {
            //TODO GUIDE
        }
        break;
    case 1: //C
        if (in == true) {
            valvecode[0] = 1 << 12 | CAM_BASE + feed * CAM_LINE_NUMBER + sxt * 2 + 0; //ban in
            *num = 1;
        } else {
            valvecode[0] = CAM_BASE + feed * CAM_LINE_NUMBER + sxt * 2 + 1; //quan out
            *num = 1;
        }
        break;
    case 2: //E
        if (in == true) {
            //ban in,quan in
            valvecode[0] = 1 << 12 | CAM_BASE + feed * CAM_LINE_NUMBER + sxt * 2 + 0; //ban in
            valvecode[1] = 1 << 12 | CAM_BASE + feed * CAM_LINE_NUMBER + sxt * 2 + 1; //quan in
            *num = 2;
        } else {
            //TODO GUIDE
        }
        break;
    default:
        break;
    }
}


static uint16 L10L12_yarnfinger2Valvecode(uint16 funcval) {
    uint16 valvecode;
    uint32 finger, feed;
    valvecode = !(funcval % 2) << 12; //in:1<<12  out:0<<12
    feed = (funcval - 0x48) / 20;               // yarn 4 feeds,  one feed Contain 10 in 10 out
    finger = (funcval - 0x48 - (feed * 20)) / 2;
    valvecode |= (YARN_FINGER_BASE + (YARN_LINE_NUMBER * feed) + finger);
    return valvecode;
}


static uint16 L10L12misc0203code2Valvecode(uint16 codevalue) {
    int16 inorout;
    int16 ivalve;
    int16 re = 0xffff;
    if ((codevalue <= 0x47) || (codevalue >= 0x98 && codevalue <= 0xa9)
        || (codevalue >= 0xb4 && codevalue <= 0xc3)) {
        inorout = !(codevalue % 2) << 12;
        ivalve = codevalue >> 1;
        re = inorout | ivalve + VALVE_MIS_BASE;
    }
    return re;
}

static void L10L12_fun0203Resolve(uint16 codevalue, uint16 *valvecode, uint32 *valnum) {
    *valnum = 0;
    if (codevalue >= 0x48 && codevalue < 0x98) {
        *valvecode = L10L12_yarnfinger2Valvecode(codevalue);
        *valnum = 1;
    } else {
        *valvecode = L10L12misc0203code2Valvecode(codevalue);
        if (*valvecode != 0xffff) {
            *valnum = 1;
        } else {
            //TODO GUIDING
        }
    }
}


static uint16 L04E7_misc0203code2Valvecode(uint16 codevalue) {
    static uint16 mis0203funmap[0x90] = {
        [0] = EV5, [1] = EV17, [2] = EV22, [3] = EV34, [4] = EV35,
        [5] = EV36, [6] = EV38, [7] = EV39, [8] = EV40, [9] = EV41,
        [10] = EV42, [11] = EV43, [12] = EV50, [13] = EV51, [14] = EV52,
        [15] = EV53, [16] = EV54, [17] = EV56, [18] = EV58, [19] = EV59,
        [20] = EV60, [21] = EV67, [22] = EV68, [23] = EV70, [26] = EV79,
        [27] = EV80,[28] = EV81, [29] = EV82, [30] = EV89, [31] = EV90,
        [32] = EV101, [77] = EV86, [78] = EV87, [81] = EV107, [82] = EV108,
        [83] = EV109, [85] = EV160, [86] = EV161,
    };

    int16 inorout;
    int16 ivalve;
    int16 re = 0xffff;
    if (codevalue >= 0xa3 && codevalue <= 0xa8) { //EV107 EV108 EV109
        codevalue--;
    }
    ivalve = codevalue >> 1;
    inorout = !(codevalue % 2) << 12;
    re = inorout | mis0203funmap[ivalve];
    return re;
}


static uint16 L04E7_yarnfinger2Valvecode(uint16 funcval) {
    uint16 valvecode = !(funcval % 2) << 12; //in:1<<12  out:0<<12
    funcval = (funcval - 0x4a) / 2;
    static uint16 selcode[40] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x0f, 0x1f, 0x2f, 0x3f, 0x0d, 0x1d, 0x2d, 0x3d, };
    valvecode |= YARN_FINGER_BASE + selcode[funcval];
    return valvecode;
}


static void L04E7_fun0203Resolve(uint16 codevalue, uint16 *valvecode, uint32 *valnum) {
    uint16 valvecode_t;
    if (codevalue >= 0x4a && codevalue <= 0x99) {
        valvecode_t = L04E7_yarnfinger2Valvecode(codevalue);
    } else {
        valvecode_t = L04E7_misc0203code2Valvecode(codevalue);
    }
    *valvecode = valvecode_t;
    *valnum = 1;
}


static uint16 misc0306code2Valvecode(uint16 codevalue) {
    uint16 inorout = !(codevalue % 2) << 12;
    uint16 ivalve = codevalue >> 1;
    return inorout | ivalve + VALVE_0603_BASE;
}



static void L10L12_funcode2Alarm(FUNC *func, uint16 *alarmcode, uint32 *alarmnum) {
    if (func->funcode == 0x031e) {
        switch (func->value) {
        case 0x01:
            alarmcode[*alarmnum] = COMMON_FUNCODE_8_CODE;
            (*alarmnum)++;
            break;
        default:
            break;
        }
    } else if (func->funcode == 0x011e) {
        switch (func->value) {
        case 0x03:
            alarmcode[*alarmnum] = COMMON_FUNCODE_4_CODE;
            (*alarmnum)++;
            break;
        default:
            break;
        }
    } else if (func->funcode == 0x0303) {
        if ((func->value <= 0x14 && func->value >= 0x10)
            || (func->value <= 0x2d && func->value >= 0x2c)) {
            alarmcode[*alarmnum] = func->value;
            (*alarmnum)++;
        }
    }
}


static void L04E7_funcode2Alarm(FUNC *func, uint16 *alarmcode, uint32 *alarmnum) {
    if (func->funcode == 0x031e) {
        switch (func->value) {
        case 0x01:
            alarmcode[*alarmnum] = COMMON_FUNCODE_8_CODE;
            (*alarmnum)++;
            break;
        default:
            break;
        }
    } else if (func->funcode == 0x0303) { //fun 12
        if (func->value == 01) {
            alarmcode[*alarmnum] = COMMON_FUNCODE_12_2d_CODE;
            (*alarmnum)++;
        }
    }
}


void L10L12_031eToValvecode(FUNC *fun, uint16 *valvecode, uint32 *valnum,
                            uint16 *alarmcode, uint32 *alarmnum) {
    if (fun->value < 0x0d) {
        L10L12_funcode2Alarm(fun, alarmcode, alarmnum);
        alarmcode += *alarmnum;
    } else if (fun->value >= 0x0d && fun->value <= 0x14) { //hafu zhen sanjiao
        *valvecode = L10L12_hafuzhencode2Valvecode(fun->value);
        *valnum = 1;
    } else if ((fun->value >= 0x65 && fun->value <= 0xa4)
               || (fun->value >= 0xc5 && fun->value <= 0x104)) { //free sel line 1 to line8
        *valvecode = L10L12_freeselcode2Valvecode(fun->value);
        *valnum = 1;
    } else if ((fun->value >= 0x1d && fun->value <= 0x64)
               || (fun->value >= 0xa5 && fun->value <= 0xc4)) { // fix sel
        L10L12_fixselcode2Valvecode(fun->value, valvecode, valnum);
    }
}


void L04E7_031eToValvecode(FUNC *fun, uint16 *valvecode, uint32 *valnum,
                           uint16 *alarmcode, uint32 *alarmnum) {
    if (fun->value == 0x01) {
        L04E7_funcode2Alarm(fun, alarmcode, alarmnum);
        alarmcode += *alarmnum;
    } else if (fun->value >= 0x02 && fun->value <= 0x09) { //hafu zhen sanjiao
        *valvecode = L04E7_hafuzhencode2Valvecode(fun->value);
        *valnum = 1;
    }
}


static void fun010fcode2Valvecode(uint16 funcode, uint16 *valvecode, uint32 *valnum) {
    uint16 valvecode_t = 0;
    *valnum = 0;
    if (funcode >= 0x0a && funcode <= 0x0b) {
        valvecode_t |= (funcode % 2) << 12;
        valvecode_t |= funcode / 2 + VALVE_1F01_BASE;
        *valvecode = valvecode_t;
        *valnum = 1;
    }
}

static void funcodeResolve(FUNC *fun, uint16 *valvecode, uint32 *valnum,
                           uint16 *alarmcode, uint32 *alarmnum, uint32 *flag) {
    //uint16 val;
    *valnum = 0;
    *alarmnum = 0;
    switch (fun->funcode) {
    case 0x031e: //sel
        //L10l12_031eToValvecode(fun, valvecode, valnum, alarmcode, alarmnum);
        machine.fun031eToValvecode(fun, valvecode, valnum, alarmcode, alarmnum);
        break;
    case 0x0302: //YARN finger
        machine.fun0203Resolve(fun->value, valvecode, valnum);
        break;
    case 0x0306:
        *valvecode = misc0306code2Valvecode(fun->value);
        *valnum = 1;
        break;
    case 0x0305:  //cam
        camcode2Valvecode(fun, valvecode, valnum);
        if (*valnum != 0) {
            *flag |= LINE_FLAG_ACT;
        }
        break;
    case 0x0300:
        *valvecode = common0506func2Valvecode(fun->value); //fun05 fun06
        *valnum = 1;
        break;
        //case 0x011e:
    case 0x0303:
        machine.funcode2Alarm(fun, alarmcode, alarmnum);
        //alarmcode += *alarmnum;
        break;
    case 0x011f:
        fun010fcode2Valvecode(fun->value, valvecode, valnum);
        break;
    default:
        break;
    }
}




/************************for co debug **************************/
#include "qifa.h"
static FIL acttestfile;
bool coActTestBegin(const TCHAR *filepath, S_CO_RUN *run) {
    if (f_open(&acttestfile, filepath, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        char filename[30], buf[100];
        wtrToStr(filename, run->co->filename);
        sprintf(buf, "co filename:%s %d steps %d lines\r\n", filename, run->numofstep, run->numofline[0]);
        uint32 wr;
        f_write(&acttestfile, buf, strlen(buf), &wr);
        return true;
    }
    return false;
}


void coActTest(S_CO_RUN_LINE *line) {
    char buf[200];
    uint32 wr;
    const char *inout;
    if (line->iline != line->co_run->stepptr[line->istep]->ilinetag[0]) {
        return;
    }
    sprintf(buf, "\t step:%3d   line:%3d\r\n\r\n", line->istep, line->iline);
    f_write(&acttestfile, buf, strlen(buf), &wr);
    for (int i = 0; i < 360; i++) {
        if (line->act[i].num == 0 && line->alarm[i].num == 0) continue;
        sprintf(buf, "\t\t\t angle %3d, act num %d,alarm num %d:", i, line->act[i].num, line->alarm[i].num);
        f_write(&acttestfile, buf, strlen(buf), &wr);
        for (int j = 0; j < line->act[i].num; j++) {
            inout = (line->act[i].valvecode[j] & 1 << 12) ? "in" : "out";
            uint16 vavle = line->act[i].valvecode[j] & ~((uint16)1 << 12);
            const char *qifanickname = qifaNickName(vavle);
            if (qifanickname == NULL) {
                qifanickname = "NULL";
            }
            sprintf(buf, "\t%s %04x %s", qifanickname, vavle, inout);
            f_write(&acttestfile, buf, strlen(buf), &wr);
        }
        for (int j = 0; j < line->alarm[i].num; j++) {
            sprintf(buf, "\t%04x %s", line->alarm[i].alarmcode[j], "alarm");
            f_write(&acttestfile, buf, strlen(buf), &wr);
        }
        sprintf(buf, "\r\n");
        f_write(&acttestfile, buf, strlen(buf), &wr);
    }
}


void coActTestEnd() {
    f_close(&acttestfile);
}














