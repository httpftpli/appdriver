#include "type.h"
#include "list.h"
#include "ff.h"
#include <string.h>
#include "pf_platform_cfg.h"
#include "mmath.h"
#include "co.h"
#include "algorithm.h"
#include "debug.h"



FUNC __func[1000];
struct list_head funcfreelist;


FENGMEN __fengmen[1000];
struct list_head fengmenfreelist;

S_CO_RUN_STEP __run_step[500];
struct list_head runstepfreelist;


void coInit() {
    INIT_LIST_HEAD(&funcfreelist);
    INIT_LIST_HEAD(&fengmenfreelist);
    INIT_LIST_HEAD(&runstepfreelist);
    for (int i = 0; i < lenthof(__func); i++) {
        list_add_tail(&__func[i].list, &funcfreelist);
    }
    for (int i = 0; i < lenthof(__fengmen); i++) {
        list_add_tail(&__fengmen[i].list, &fengmenfreelist);
    }
    for (int i = 0; i < lenthof(__run_step); i++) {
        list_add_tail(&__run_step[i].list, &runstepfreelist);
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
    unsigned int size,  br;
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
        FUNC *func  = list_entry(funcfreelist.next, FUNC, list);
        while (1) {
            func  = list_entry(funcfreelist.next, FUNC, list);
            r = f_read(file, func, 2, &br);
            if (func->angular != 0x8000) {
                r = f_read(file, &func->angular, 4, &br);
                if (func->unknow == 0x031f) {
                    r = f_read(file, &func->add, 4, &br);
                } else if (func->unknow == 0x030b) {
                    r = f_read(file, &func->add, 2, &br);
                }
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
            fengmen  = list_entry(fengmenfreelist.next, FENGMEN, list);
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

static bool readEconomizer(FIL *file, ECONOMIZER_PARAM *econo, unsigned int *num) {
    //uint32 offset= f_tell(file);
    CO_ECONOMIZER_HEAD econo_head;
    CO_ECONOMIZER_PARAM param;
    uint32 br;
    FRESULT r = f_read(file, &econo_head, sizeof econo_head ,&br);
    if (r != FR_OK || br != sizeof econo_head) {
        return false;
    }
    ASSERT(econo_head.flag == 0xff);
    if (econo_head.flag != 0xff) {
        return false;
    }
    uint32 numtemp = (econo_head.size - econo_head.headsize) / sizeof(CO_ECONOMIZER_PARAM);
    *num = numtemp;
    for (int i = 0; i < numtemp; i++) {
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
    r = f_read(file, &numtemp, 4 ,&br);
    if (r != FR_OK || br != 4 || numtemp!=0) {
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



bool coParas(const TCHAR *path, S_CO *co, unsigned int *offset) {
    FIL file;
    unsigned int br;


    //do co struct init
    for (int i = 0; i < lenthof(co->func); i++) {
        INIT_LIST_HEAD(&co->func[i]);
    }
    for (int i = 0; i < lenthof(co->fengmen); i++) {
        INIT_LIST_HEAD(&co->fengmen[i]);
    }
    //open co file
    FRESULT  r = f_open(&file, path, FA_READ);
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

    CO_PART2_ATTRIB *coattri2 =  (CO_PART2_ATTRIB *)buf;
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
    coattri4->fengmen2addr +=  coPart4Offset;
    coattri4->fengmen1addr +=  coPart4Offset;
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
    //read economizers
    if (!readEconomizer(&file, co->econo, &co->numofeconomizer)) {
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



static void  cocreateindex_speed(S_CO_RUN *co_run, S_CO *co){
    uint32  ispeed = 0;
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (co->speed[ispeed].step == i) {
            step->speed = &co->speed[ispeed++];
        } else {
            step->speed = NULL;
        }
    }
}



static void  cocreateindex_econ(S_CO_RUN *co_run, S_CO *co) {
    uint32 iecon = 0;
    if (co->numofeconomizer==0) {
        for (int i = 0; i < 8; i++) {
             co_run->numofline[i] = co->numofstep;
        }
        for (int i=0;i<co->numofstep;i++) {
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
            for (int i = 0; i < 8; i++) {
                co_run->numofline[i] += econodif * co->econo[iecon].economize[i];
            }
            iecon++;
        } else {
            step->econo = NULL;
            step->econoFlag = 0;
            for (int i = 0; i < 8; i++) {
                co_run->numofline[i]++;
            }
        }
    }
}


static void cocreateindex_sizemotor(S_CO_RUN *co_run, S_CO *co) {
    uint32  isizemotorzone = 0;
    for (int i = 0; i < co->numofstep; i++) {
        S_CO_RUN_STEP *step = co_run->stepptr[i];
        if (co->sizemotor[isizemotorzone].head.beginStep >= i && co->sizemotor[isizemotorzone].head.endStep <= i) {
            step->sizemotor = &co->sizemotor[isizemotorzone];
            if (co->sizemotor[i].head.endStep == i) {
                isizemotorzone++;
            }
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

    //economizer
    cocreateindex_econ(co_run, co);

    //sizemotor;
    cocreateindex_sizemotor(co_run, co);


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
}



uint32 corunReadLine(S_CO_RUN *co_run, S_CO_RUN_LINE *line, uint32 size) {
    uint32 stepindex = co_run->istep = co_run->nextstep;
    co_run->prerpm = co_run->rpm;
    S_CO_RUN_STEP *step = co_run->stepptr[stepindex];

    //calculate speed;
    uint32 acc;
    if (step->speed != NULL) {
        uint32 tarrpm = step->speed->rpm;
        uint32 ramp = step->speed->ramp[size];
        acc = (tarrpm - co_run->prerpm) / ramp;
        co_run->speedAcc = acc;
        co_run->targetSpeed = tarrpm;
    }

    co_run->rpm = co_run->prerpm + co_run->speedAcc;
    if (co_run->speedAcc < 0 && co_run->rpm < co_run->targetSpeed) {
        co_run->rpm = co_run->targetSpeed;
    } else if (co_run->speedAcc > 0 && line->rpm > co_run->targetSpeed) {
        co_run->rpm = co_run->targetSpeed;
    }
    if (co_run->rpm = co_run->targetSpeed) {
        co_run->speedAcc = 0;
    }

    //process step
    if (IS_ECONO_BEGIN(*step)) { // loop begin
        co_run->iecono++;
        co_run->econonum = step->econo->economize[size];
        co_run->econostepfrom = step->econo->begin;
        co_run->econostepto = step->econo->end;
    }
    if (IS_ECONO_END(*step)) { //loop end
        if (co_run->iecono  == co_run->econonum) { //end loop
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
    line->econoend = co_run->econostepto ;
    line->iline = co_run->nextline - 1;
    line->istep = stepindex;

    return co_run->numofline[size] - line->iline - 1;
}





