

#define QIFA_USE_MAILBOX   0

#include "can_wp.h"
#include "pf_can.h"
#include "qifa.h"
#include "qifacode.h"
#include "module.h"
#include "debug.h"
#include <string.h>
#include "atomic.h"
#include "pf_timertick.h"
#include "mmath.h"
#include "ff.h"
#include "mem.h"
#include "misc.h"
#include "machine_cfg.h"
#include <stdlib.h>

#if QIFA_USE_MAILBOX==1
    #include "pf_mailbox.h"
#endif




//qifa
#define CAN_WP_FUNCODE_QF_POWERSAVE     			0x21		//设置气阀节能
#define CAN_WP_FUNCODE_QF_EN            				0x22		//气阀使能
#define CAN_WP_FUNCODE_QF_RESET         			0x23		//气阀复位
#define CAN_WP_FUNCODE_QF_SETDUTY       			0x24		//气阀占空比
#define CAN_WP_FUNCODE_QF_WRITE         			0x25		//气阀输出
#define CAN_WP_FUNCODE_QF_READOHM       			0x2c		//读取气阀阻值
#define CAN_WP_FUNCODE_QF_SETALARMCODE  		0x26		//设定气阀报警真值   报警信号在正常情况状态
#define CAN_WP_FUNCODE_QF_SETALARMMASK  		0x27		//报警屏蔽位  为0屏蔽 为1 需检测信号
#define CAN_WP_FUNCODE_QF_READALARM     			0x28		//读取报警信号
#define CAN_WP_FUNCODE_QF_READ          				0x29		//气阀状态读入
#define CAN_WP_FUNCODE_QF_ALARM         			0x2b		//气阀报警(气阀板主动上发)


#define QIFA_ID(ID)    (CAN_WP_ID(CAN_WP_DEV_TYPE_QIFA,ID))
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static atomic wp_read_qf_flag,wp_qf_resdata;
static unsigned short Sw_Qf_Status;
uint16 qifawarn[30];



void wpQfPowrSave(unsigned char id, bool on) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_POWERSAVE;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)on;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfEn(unsigned char id, bool on) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_EN;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)on;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfRst(unsigned char id, bool rst) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_RESET;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)(!!rst);
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfSetDuty(unsigned char id, unsigned char duty) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_SETDUTY;
    frame.desid = QIFA_ID(id);
    frame.data[0] = (unsigned char)duty;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}



void wpQfWrite(unsigned char id, unsigned char *val, unsigned char count) {
    ASSERT(count < 9);
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_WRITE;
    frame.desid = QIFA_ID(id);
    memcpy(frame.data, val, count);
    frame.dlc = count;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}



bool wpQfReadOhm(unsigned char id, unsigned char qifaId, unsigned short timeout, unsigned short *Ohm) {
    QIFA *qifa = qifaSys.QiFa_Reg_Table[id - 1][qifaId];
    if (qifa==NULL) {
        return false;
    }
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_READOHM;
    frame.desid = QIFA_ID(id);
    frame.data[0] = qifaId | (1 << 7);
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&wp_qf_resdata);
    withintimedo(timeout) {
        if (atomicTestClear(&wp_qf_resdata)) {
            *Ohm = qifa->Ohm;
            return true;
        }
    }
    return false;
}

void wpQfSetAlarmCode(unsigned char id, unsigned char code) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_SETALARMCODE;
    frame.desid = QIFA_ID(id);
    frame.data[0] = code;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}

void wpQfSetAlarmMask(unsigned char id, unsigned char mask) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_SETALARMMASK;
    frame.desid = QIFA_ID(id);
    frame.data[0] = mask;
    frame.dlc = 1;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}


void wpQfReadAlarm(unsigned char id) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_READALARM;
    frame.desid = QIFA_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
}



bool wpQfRead(unsigned char id, unsigned int timeout, unsigned int *status) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_QF_READ;
    frame.desid = QIFA_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    atomicClear(&wp_read_qf_flag);
    withintimedo(timeout) {
        if (atomicTestClear(&wp_read_qf_flag)) {
            *status = Sw_Qf_Status;
            return true;
        }
    }
    return false;
}




void canQfRcv(CAN_WP *frame) {
    //if (frame->flag != 1 ) return;
    if (CAN_WP_GET_TYPE(frame->srcid) != CAN_WP_DEV_TYPE_QIFA) {
        return; //device id is not qifa
    }
    unsigned int funcode = frame->funcode;
    unsigned char qfban;
    switch (funcode) {
    case CAN_WP_FUNCODE_QF_READOHM:
        {
            atomicSet(&wp_qf_resdata);
            qfban = CAN_WP_GET_ID(frame->srcid) - 1;
            unsigned char qf = (unsigned char)(frame->data[0]);
            if (qfban >= qifaSys.numofboard || qf >= qifaSys.numperboard) return;
            qifaSys.QiFa_Reg_Table[qfban][qf]->Ohm = (unsigned short)(frame->data[0] >> 8);
        }
        break;
    case CAN_WP_FUNCODE_QF_READALARM:
        break;
    case CAN_WP_FUNCODE_QF_READ:
        atomicSet(&wp_read_qf_flag);
        Sw_Qf_Status = (unsigned short)(frame->data[0]);
        break;
    case CAN_WP_FUNCODE_QF_ALARM:
        qfban = CAN_WP_GET_ID(frame->srcid) - 1;
        if (qfban >= qifaSys.numofboard) return;
        qifawarn[qfban] = (uint16)(frame->data[0] & 0x0f);
        break;
    default:
        break;
    }
}



static RINGBUF qifaringbuf;
static QIFA_VAL qifavalbuf[400];



QIFA_SYS qifaSys;



#if QIFA_USE_MAILBOX==1

static void qifa_mailbox_handler(uint32 msg) {
    if (msg != 0x55555555) {
        return;
    }
    qifaProcess();
}

#endif





typedef struct {
    unsigned short flag;
    unsigned char board_id;
    unsigned char xuhao;
    unsigned int qfId;
    unsigned int default_nc_no : 1;
    unsigned int nc_no_changeable : 1;
    unsigned int nc_no_dis : 1;
    unsigned int iscam : 1;
    unsigned int qifa_name_offset[13];
}QF_PACKED;





#define QIFA_FLAG    0xaa55
#define QIFA_FLAG_EN    0xaa55
#define QIFA_FLAG_DIS   0x55aa



static OS_MEM qifamem;

static bool __qifainit(QIFA_SYS *qifasys,TCHAR *path) {
    static wchar __qifaname[20000];
    static QIFA __QiFa_Reg[160];
    for (int i=0;i<lenthof(__QiFa_Reg);i++) {
        __QiFa_Reg[i].flag = 0;
    }
    MEM_ERR mem_err;
    if (!(qifasys->flag & 0x01)) {
        MemCreate(&qifamem, "qifa mem pool", __QiFa_Reg,
                   lenthof(__QiFa_Reg),sizeof(QIFA),&mem_err);
        ASSERT(mem_err==MEM_ERR_NONE);
        qifasys->flag |= 0x01;
    }else{
        for (int i=0;i<lenthof(qifasys->QiFa_Reg);i++) {
            if (qifasys->QiFa_Reg[i]!=NULL) {
                uint32 iboard = qifasys->QiFa_Reg[i]->board_id;
                uint32 iqf = qifasys->QiFa_Reg[i]->xuhao;
                qifasys->QiFa_Reg_Table[iboard][iqf] = NULL;
                memset(qifasys->QiFa_Reg[i],0,sizeof(QIFA));
                MemPut(&qifamem,qifasys->QiFa_Reg[i],&mem_err);
                ASSERT(mem_err==MEM_ERR_NONE);
                qifasys->QiFa_Reg[i] = NULL;
            }
        }
    }

    FIL file;
    uint32 rb;
    //int nameoffset;
    MD5_CTX md5;
    MACHI_CFG_FILE_HEAD filehead;
    MACHI_CFG_HEAD *head;
    QF_PACKED *qifapack;
    unsigned char *p, *data=NULL,*dataend;
    unsigned char md5hash[16];
    if (f_open(&file, path, FA_READ) != FR_OK) {
        return false;
    }
    if (f_read(&file, &filehead, sizeof filehead,&rb) != FR_OK || rb != sizeof filehead) {
        goto ERROR;
    }
    if (strcmp(filehead.cfghead, "swjcfg") != 0) goto ERROR;

    if (f_lseek(&file, 0x80) != FR_OK) goto ERROR;
    data = (unsigned char *)malloc(f_size(&file) - 0x80);
    if (data == NULL) {
        goto ERROR;
    }
    dataend = data + f_size(&file) - 0x80;
    unsigned int rd;
    if(f_read(&file,data,f_size(&file) - 0x80,&rd)!=FR_OK || rd != f_size(&file)-0x80){
        goto ERROR;
    }
    MD5Init(&md5);
    MD5Update(&md5, data, f_size(&file) - 0x80);
    MD5Final(&md5, md5hash);
    if (memcmp(&md5hash, &filehead.md5,16 ) != 0) goto ERROR;

    p = data;
    head = (MACHI_CFG_HEAD *)p;
    if (sizeof qifapack > head->sizeofQifa) {
        goto ERROR;
    }
    qifasys->numofboard = head->numofBoard;
    qifasys->numperboard = head->numPerBoard;
    qifasys->numofqifa = head->numofQifa;
    ASSERT(qifasys->numofboard<=lenthof(qifasys->QiFa_Reg_Table));
    ASSERT(qifasys->numperboard<=lenthof(qifasys->QiFa_Reg_Table[0]));

    p += head->qifanameOffset-0x80;
    ASSERT(p<=dataend);
    memcpy(__qifaname,p,head->qifanamesize);
    p = data + head->qifaOffset-0x80;

    for (int i = 0; i < qifasys->numofqifa; i++) {
        qifapack = (QF_PACKED *)p;
        p += sizeof(QF_PACKED);
        if (qifapack->flag != QIFA_FLAG_EN || qifapack->flag != QIFA_FLAG_DIS) {
            goto ERROR;
        }
        if (qifapack->flag == QIFA_FLAG_DIS) {
            continue;
        }
        QIFA *qifa = MemGet(&qifamem, &mem_err);
        ASSERT(mem_err==MEM_ERR_NONE);
        uint16 qfid = qifapack->qfId;
        qifa->board_id = qifapack->board_id;
        qifa->xuhao = qifapack->xuhao;
        qifa->nc_no = qifapack->default_nc_no;
        qifa->default_nc_no = qifapack->default_nc_no;
        qifa->nc_no_changeable = qifapack->nc_no_changeable;
        qifa->nc_no_display = qifapack->nc_no_dis;
        qifa->cam_en = qifapack->iscam;
        qifa->flag = QIFA_FLAG;
        qifa->qfid = qfid;
        for (int j=0;j<13;j++) {
            if (qifapack->qifa_name_offset[j]!=-1UL) {
                qifa->name[j] = &__qifaname[qifapack->qifa_name_offset[j] / 2];
            }else{
                qifa->name[j] = &__qifaname[qifapack->qifa_name_offset[0] / 2];
            }
        }
        char nicknamebuf[100];
        wtrToStr(nicknamebuf,qifa->name[0]);
        strtok(nicknamebuf,"[");
        char *p =  strtok(nicknamebuf,"]");
        strcpy(qifa->nickname,p+1);

        qifasys->QiFa_Reg[qfid] = qifa;
    }
    f_close(&file);
    if (data!=NULL) {
        free(data);
    }
    return true;
    ERROR:
    f_close(&file);
    if (data!=NULL) {
        free(data);
    }
    return false;
}



bool qifaInit(TCHAR *path) {
    if (__qifainit(&qifaSys,path) == false) {
        return false;
    }
    memset(qifaSys.QiFa_Reg_Table,0, sizeof qifaSys.QiFa_Reg_Table);
    for (uint32 i = 0; i < lenthof(qifaSys.QiFa_Reg); i++) {

        if (qifaSys.QiFa_Reg[i] == NULL || qifaSys.QiFa_Reg[i]->flag != QIFA_FLAG ) {
            continue;
        }
        uint32 board = qifaSys.QiFa_Reg[i]->board_id;
        uint32 iqifa = qifaSys.QiFa_Reg[i]->xuhao;
        if (board >= qifaSys.numofboard || iqifa >= qifaSys.numperboard) {
            return false;
        }
        qifaSys.QiFa_Reg_Table[board][iqifa] = qifaSys.QiFa_Reg[i];
    }

    ringBufInit(&qifaringbuf, qifavalbuf, sizeof(QIFA_VAL), lenthof(qifavalbuf), 1);

#if QIFA_USE_MAILBOX==1
    mailboxInit(MODULE_ID_MB);
    uint32 mbaddr = modulelist[MODULE_ID_MB].baseAddr;
    MBenableNewMsgInt(mbaddr, 0, 0);
    mbRegistHandler(0,qifa_mailbox_handler);
#endif
    //////////////////////////////////////

    return true;
}


const wchar * qifaName(QIFA *qifa ,uint32 nameindex) {
    if (qifa == NULL || qifa->flag!=QIFA_FLAG) {
        return NULL;
    }
    if(nameindex>12) nameindex = 0;
    return qifa->name[nameindex];
}


const char *qifaNickName(uint16 valecode){
    QIFA *qifa = qifaSys.QiFa_Reg[valecode];
    if (qifa && qifa->flag==QIFA_FLAG) {
        return qifa->nickname;
    }
    return NULL;
}


void qifaSet(QIFA *qifa, uint32 val){
    if (qifa == NULL || qifa->flag!=QIFA_FLAG) {
        return;
    }
    QIFA_VAL qifa_val;
    qifa_val.qifa = qifa;
    qifa_val.val = !!val;
    while (!ringBufPush(&qifaringbuf, &qifa_val));
#if QIFA_USE_MAILBOX == 1
    uint32 mbaddr = modulelist[MODULE_ID_MB].baseAddr;
    MBsendMessage(mbaddr, 0, 0x55555555);
#endif
}

void qifaFunSet(uint16 funcode,uint32 val){
    if (funcode==NUd) {
        return;
    }
    QIFA *qifa = qifaSys.QiFa_Reg[funcode];
    qifaSet(qifa, val);
}


void qifaSet1(uint32 wpId, uint32 iqifa, uint32 val) {
    ASSERT(wpId > 0 && wpId <= qifaSys.numofboard);
    QIFA *qifa = qifaSys.QiFa_Reg_Table[wpId - 1][iqifa];
    qifaSet(qifa, val);
}


void qifaSetIo(uint32 wpId, uint32 iqifa, uint32 val) {
    ASSERT(wpId > 0 && wpId <= qifaSys.numofboard);
    QIFA *qifa = qifaSys.QiFa_Reg_Table[wpId - 1][iqifa];
    if (qifa==NULL) {
        return;
    }
    qifaSet(qifa, !!val ^ qifa->nc_no);
}



int32 qifaRead(QIFA * qifa, bool comm) {
    if (qifa==NULL) {
        return 0;
    }
    ASSERT(qifa->flag == QIFA_FLAG);
    return qifa->inout;
}


int32 qifaRead1(uint32 wpId, uint32 iqifa, bool comm) {
    ASSERT(wpId > 0 && wpId <= qifaSys.numofboard);
    QIFA *qifa = qifaSys.QiFa_Reg_Table[wpId - 1][iqifa];
    if (qifa==NULL) {
        return 0;
    }
    return qifa->inout;
}


uint32 qifaRead2(uint32 wpId, bool comm) {
    uint32 val = 0;
    ASSERT(wpId > 0 && wpId <= qifaSys.numperboard);
    for (int i = 0; i < qifaSys.numperboard; i++) {
        val |= (!!qifaRead1(wpId,1,0)) << i;
    }
    return val;
}


int32 qifaReadIo(QIFA * qifa, bool comm) {
    if (qifa==NULL) {
        return 0;
    }
    ASSERT(qifa->flag == QIFA_FLAG);
    return(!!qifa->inout) ^ (!!qifa->nc_no);
}

int32 qifaReadIo1(uint32 wpId, uint32 iqifa, bool comm) {
    ASSERT(wpId > 0 && wpId <= qifaSys.numofboard);
    QIFA *qifa = qifaSys.QiFa_Reg_Table[wpId - 1][iqifa];
    return qifaReadIo(qifa, comm);
}

uint32 qifaReadIo2(uint32 wpId, bool comm) {
    uint32 val = 0;
    ASSERT(wpId > 0 && wpId <= qifaSys.numofboard);
    for (int i = 0; i < qifaSys.numperboard; i++) {
        val |= (!!qifaReadIo1(wpId, i, comm)) << i;
    }
    return val;
}



/*uint16 qifaReadWarn(uint32 wpId){
    uint16 tmp = qifawarn[wpId-1];
    if(tmp!=0){
        qifawarn[wpId-1] = 0;
    }
    return tmp;
}*/


uint32 qifaBak(QIFA * qifa) {
    if (qifa == NULL || qifa->flag!=QIFA_FLAG) {
        return -1UL;
    }
    return qifa->inout_bak = qifa->inout;
}

uint32 qifaBak2(uint32 wpId) {
    uint32 val = 0;
    for (int i = 0; i < qifaSys.numperboard; i++) {
        val |= qifaBak(qifaSys.QiFa_Reg_Table[wpId - 1][i]) << i;
    }
    return val;
}


void qifaRestore(QIFA * qifa) {
    if (qifa == NULL || qifa->flag!=QIFA_FLAG) {
        return;
    }
    qifaSet(qifa, qifa->inout_bak);
}


void qifaRestore2(uint32 wpId) {
    for (int i = 0; i < qifaSys.numperboard; i++) {
        qifaRestore(qifaSys.QiFa_Reg_Table[wpId - 1][i]);
    }
}

#define QIFA_BOARD_NUM_MAX        20


#define QIFA_GUILEI_BUF_LEN  200
typedef struct {
    unsigned char data_value[QIFA_GUILEI_BUF_LEN];
    int num;
}Qifa_Guilei;



void qifaProcess() {
    static Qifa_Guilei qifaguilei[QIFA_BOARD_NUM_MAX];
    QIFA_VAL qifaval;
    QIFA *qifa;
    int val;
    while (1) {
        if (!ringBufPop(&qifaringbuf, &qifaval)) {
            break;
        }
        qifa = qifaval.qifa;
        ASSERT(qifa!=NULL && qifa->flag == QIFA_FLAG);
        val = qifa->inout = qifaval.val;
        unsigned int id = qifa->board_id;
        ASSERT(id < qifaSys.numofboard);
        qifaguilei[id].data_value[qifaguilei[id].num]
        = (uint8)((val ^ !!qifa->nc_no) << 7 | qifa->xuhao);
        qifaguilei[id].num++;
        ASSERT(qifaguilei[id].num < QIFA_GUILEI_BUF_LEN);
    }
    int32 j;
    for (int i = 0; i < qifaSys.numofboard; i++) {
        if (qifaguilei[i].num != 0) {
            j = 0;
            do {
                uint32 datanum = MIN(qifaguilei[i].num, 8);
                wpQfWrite(i + 1, &qifaguilei[i].data_value[j], datanum);
                j += datanum;
                qifaguilei[i].num -= datanum;
            }while (qifaguilei[i].num);
        }
    }
}
