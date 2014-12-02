
typedef struct {
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
    uint32 unknow;
}CO_PART1_ATTRIB;


typedef struct {
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
}CO_PART2_ATTRIB;


typedef struct {
    char unknow[40];
    uint32 func_addr;
    char unknow1[8];
    uint32 speed_addr;
    char unknow2[8];
}CO_PART3_ATTRIB;


typedef struct {
    char unknow[40];
    uint32 func_addr;
    char unknow1[8];
    uint32 speed_addr;
    char unknow2[8];
}CO_PART3_ATTRIB;

typedef struct {
    uint16 beginFlag; //0xffff
    uint16 angle;
    uint16 beginStep;
    uint16 endStep;
    uint16 groupNum;
    uint16 unkown;  //0x0000
}CO_MOTORHEAD;

typedef struct {
    uint16 beginFlag; //0xffff
    uint16 angle;
    uint16 beginStep;
    uint16 endStep;
    uint16 groupNum;
    uint16 unkown;  //0x0000
}CO_MOTORHEAD;

typedef struct {
    uint16 beginStep;
    uint16 endStep;
    uint16 angle;
    uint16 groupNum;
}MOTORHEAD;

typedef struct {
    uint16 start[3];     //motor duplicate 3 times
    uint16 startWidth;
    uint16 startWidthDec; //dec: 0-0,1-0.25 3-0.75
    uint16 end[3];  //cylinder duplicate 3 times
    uint16 endWidth;
    uint16 endWidthDec; //dec: 0-0,1-0.25 3-0.75
}CO_SIZEMOTORPARAM;


typedef struct {
    uint16 start;     //motor duplicate 3 times
    uint16 startWidth;
    uint16 startWidthDec; //dec: 0-0,1-0.25 3-0.75
    uint16 end;  //cylinder duplicate 3 times
    uint16 endWidth;
    uint16 endWidthDec; //dec: 0-0,1-0.25 3-0.75
}SIZEMOTORPARAM;

typedef struct {
    unsigned int size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    CO_MOTORHEAD head;
    CO_SIZEMOTORPARAM param[8];
}CO_SIZEMOTOR_ZONE;


typedef struct {
    char descrpition[24];
    MOTORHEAD head;
    SIZEMOTORPARAM param[8];
    struct list_head list;
}SIZEMOTOR_ZONE;



typedef struct {
    uint16 qi_feed[3];
    char unknow[4];
    uint16 qf_feed[3];
    char unknow1[4];
}CO_SINKERMOTORPARAM;

typedef struct {
    uint32 size;
    unsigned int prev; //0x00000000
    char descrpition[24];
    MOTORHEAD head;
    CO_SINKERMOTORPARAM param[8];
}CO_SINKERMOTOR_ZONE;


typedef struct {
    uint16 qi_feed;
    uint16 qf_feed;
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


typedef struct {
    unsigned short num;
    unsigned short unkown;
    MOTOR_HEADER_PARAM param[num];
}CO_MOTOR_HEADER_GROUP;




typedef struct {
    uint32 step;
    uint32 rpm[2];
    uint16 ramp[8][2];
}CO_SPEED;


typedef struct {
    uint32 step;
    uint32 rpm;
    uint16 ramp[8];
    struct list_head list;
}SPEED;


typedef struct {
    uint16 angular;
    uint16 unknow;
    uint16 add[4];
    struct list_head list;
}FUNC;

FUNC func[1000];
struct list_head funcfreelist;

typedef struct {
    uint32 diameter;
    uint16 niddle;
    uint32 numofspeed;
    uint32 numofstep;
    SPEED speed[100];
    uint32 numofsizemotorzone;
    SIZEMOTOR_ZONE  sizemotor[20];
    uint32 numofsinkmoterzone_1_3;
    SINKERMOTOR_ZONE  sinkmoterzone_1_3[20];
    uint32 numofsinkmoterzone_2_4;
    SINKERMOTOR_ZONE  sinkmoterzone_2_4[20];
    uint32 numofsinkangular;
    SINKERMOTOR_ZONE  sinkangular[20];
    struct list_head func[500];
    //sink sinkermotor
}CO_RUN;


typedef struct {
    uint16 size;
    uint16 istep;
    uint32 iring;
    uint32 iring_in_step;
    SPEED *speed;
    SIZEMOTOR_ZONE *sizemotor;
    uint32 rpm;
}RUN_PARAM;



static bool readSinkMotor(FIL *file  SINKERMOTOR_ZONE *zone, unsigned int *num) {
    *num = 0;
    while (1) {
        CO_SIZEMOTOR_ZONE co_zone;
        r = f_read(&file, &co_zone, sizeof co_zone,&br);
        if (r != FR_OK || br != sizeof co_zone) {
            //re = CO_FILE_READ_ERROR;
            return false;
        }
        if (co_zone.size != 0) {
            SIZEMOTOR_ZONE *zonetemp = zone + *num;

            memcpy(&zonetemp->descrpition, &co_zone.descrpition, sizeof zonetemp->descrpition);

            zonetemp->head.angle = co_zone.head.angle;
            zonetemp->head.beginStep = co_zone.head.beginStep;
            zonetemp->head.endStep = co_zone.head.endStep;
            zonetemp->head.groupNum = co_zone.head.groupNum;

            for (int i = 0; i < 8; i++) {
                zonetemp->param[i].start = co_zone.param[i].start[0];
                zonetemp->param[i].startWidth = co_zone.param[i].startWidth;
                zonetemp->param[i].startWidthDec = co_zone.param[i].startWidthDec;
                zonetemp->param[i].end = co_zone.param[i].end[0];
                zonetemp->param[i].endWidth = co_zone.param[i].endWidth;
                zonetemp->param[i].endWidthDec = co_zone.param[i].endWidthDec;
            }
            num++;
        } else {
            f_lseak(&file, f_tell() - sizeof co_zone + 4); //back filepoint
            break;
            //TODO: numof sizemotor zone protect
        }
    }
    return true;
}



static bool readSizemotor(FIL *file  SIZEMOTOR_ZONE *zone, unsigned int *num) {
    *num = 0;
    while (1) {
        CO_SIZEMOTOR_ZONE co_szone;
        r = f_read(&file, &co_szone, sizeof co_szone,&br);
        if (r != FR_OK || br != sizeof co_szone) {
            //re = CO_FILE_READ_ERROR;
            return false;
        }
        if (co_szone.size != 0) {
            SIZEMOTOR_ZONE *zone_temp = zone + *num;
            memcpy(&zone_temp->descrpition, &co_szone.descrpition, sizeof szone->descrpition);

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
            *num++;
        } else {
            f_lseak(&file, f_tell() - sizeof co_szone + 4); //back filepoint
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
        r = f_read(&file, &co_speed, sizeof co_speed,&br);
        if (r != FR_OK || br != sizeof co_speed) {
            //re = CO_FILE_READ_ERROR;
            return false;
        }
        if (co_speed.step != 0xffffffff) {
            SPEED *speedtemp = &co->speed[*num];
            speed->ramp = co_speed.ramp;
            speed->rpm = co_speed.ramp;
            speed->step = co_speed.step;
            *num++;
        } else {
            f_lseak(&file, f_tell() - sizeof co_speed + 4); //back filepoint
            break;
            //TODO: numof SPEED protect
        }
    }
    return true;
}


bool readFunc(FIL *file, struct list_head *funclist, unsigned int *step) {
    *num = 0;
    unsigned fileoffset = f_tell(file);
    unsigned int size;
    r = f_read(&file, &size, sizeof size,&br);
    if (r != FR_OK || br != sizeof size) {
        //re = CO_FILE_READ_ERROR;
        return false;
    }
    f_lseek(file, fileoffset);
    *step = size / 4;
    for (int i = 0; i < *step; i++) {
        unsigned int addr;
        //read addr
        r = f_read(&file, &addr, 4, &br);
        if (r != FR_OK || br != 4) {
            //re = CO_FILE_READ_ERROR;
            return false;
        }
        addr += fileoffset;
        f_lseek(file, addr);
        //read func;
        FUNC *func  = list_entry(funcfreelist->next,FUNC,list);
        while (1) {
            r = f_read(&file, func, 2, &br);
            if (func->angular != 0x8000) {
                r = f_read(&file, &func->angular, 4, &br);
                if (func->funcode == 0x031f) {
                    r = f_read(&file, &func->.add, 4, &br);
                } else if (func->funcode == 0x030b) {
                    r = f_read(&file, &func->add, 2, &br);
                }
                list_move_tail(&func->list,&funclist[i]);
            } else {
                break;
            }
        }
    }
    return true;
}

bool parasCo(const TEXTCHAR path, CO_RUN *co, unsigned int &offset) {
    FIL file;
    unsigned int re;
    FRESULT  r = f_open(&file, path, FA_READ);
    if (r != FR_OK) {
        //re = CO_FILE_OPEN_ERROR;
        goto ERROR;
    }
    CO_PART1_ATTRIB coattri1;
    unsigned char buf[512];
    unsigned int n;
    f_lseek(&file, 512);
    r = f_read(&file, &coattri1, sizeof coattri);
    if (r != FR_OK || n != sizeof coattri) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }

    co->diameter = coattri.diameter;
    co->niddle = coattri.niddle;
    r = f_read(&file, buf, sizeof CO_ATTRIB &n);
    if (r != FR_OK || n != sizeof CO_ATTRIB) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    //justify address ;
    coattri1.sizemotorAddr += 512;
    coattri1.sinkermotor_feed1_3_Addr += 512;
    coattri1.stitch1Addr += 512;
    coattri1.stitch2Addr += 512;
    coattri1.stitch3Addr += 512;
    coattri1.stitch4Addr += 512;
    coattri1.sinkermotor_feed2_4_Addr += 512;
    coattri1.sinkerangular_Addr += 512;
    //read sizemoter;
    f_lseek(&file, coattri.sizemotorAddr);
    readSizemotor(&file, &co->sizemotor, &co->numofsizemotorzone);

    //read sinkermotor_feed1_3;
    f_lseek(&file, &coattri.sinkermotor_feed1_3_Addr);
    if (!readSinkMotor(FIL * file, &co->sinkmoterzone_1_3, &co->numofsinkmoterzone_1_3)) {
        goto ERROR;
    }
    //read sinkermotor_feed2_4;
    f_lseek(&file, coattri.sinkermotor_feed2_4_Addr);
    if (!readSinkMotor(FIL *file,->sinkmoterzone_2_4,&co->numofsinkmoterzone_2_4)) {
        goto ERROR;
    }
    //read sinkerangular;
    f_lseek(&file, coattri.sinkerangular_Addr);
    if (readSinkMotor(FIL * file, &co->sinkangular, &co->numofsinkangular)) {
        goto ERROR;
    }


    //512 aligne
    uint32 coPart2Offset = BOUNDUP(512 + coattri.co_size, 512);
    f_lseek(&file, coPart2Offset);
    //read part2 of CO ,including chain;

    CO_PART2_ATTRIB coattri2;
    r = f_read(&file, &coattri2, sizeof coattri2);
    if (r != FR_OK || br != 4) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    coattri2.unkownaddr[11] += coPart2Offset;
    f_lseek(&file, coPart2Offset + coattri2.unkownaddr[11]);
    uint32 lastmotor;
    r = f_read(&file, &lastmotor, sizeof lastmotor,&br);
    if (r != FR_OK || br != 4) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }
    if (lastmotor != 0) {
        return CO_FILE_READ_ERROR;
    }
    //read MOTOR_HEADER_GROUP
    MOTOR_HEADER_GROUP  motor_header_group;
    r = f_read(&file, &motor_header_group, sizeof motor_header_group,&br);
    if (r != FR_OK || br != sizeof motor_header_group) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }

    coPart3Offset = BOUNDUP(f_tell(&file), 256);
    f_lseek(&file, coPart3Offset);

    //read part 3 of co
    CO_PART3_ATTRIB coattri3;
    r = f_read(&file, &coattri3, sizeof coattri3,&br);
    if (r != FR_OK || br != sizeof coattri3) {
        //re = CO_FILE_READ_ERROR;
        goto ERROR;
    }

    coattri3.speed_addr += coPart3Offset;
    coattri3.func_addr += coPart3Offset;

    f_lseek(&file, coattri3.speed_addr);
    if (!readSpeed(&file, co->speed, &co->numofspeed)) {
        goto ERROR;
    }

    //read funcode;
    f_lseek(&file, coattri3.func_addr);
    unsigned int step;
    readFunc(&file, co->func,&step );
    co->numofstep = step;

    //read part 4 of co
    CO_PART4_ATTRIB coattri4;

    f_close(&file);
    return true;

    ERROR:
    offset = f_tell(&file);
    f_close(&file);
    return false;
}



