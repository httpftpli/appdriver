
#ifndef __CAN__WP__H__
#define __CAN__WP__H__


#define CAN_WP_DEV_TYPE_MAIN    0x10
#define CAN_WP_DEV_TYPE_MOTOR   0x20
#define CAN_WP_DEV_TYPE_INPUT   0x30
#define CAN_WP_DEV_TYPE_QIFA    0x40
#define CAN_WP_DEV_TYPE_SIFU    0x50



//motor
#define CAN_WP_FUNCODE_MOTOR_POWERSAVE     0x21
#define CAN_WP_FUNCODE_MOTOR_EN            0x22
#define CAN_WP_FUNCODE_MOTOR_RESET         0x23
#define CAN_WP_FUNCODE_MOTOR_SETDUTY       0x24
#define CAN_WP_FUNCODE_MOTOR_WRITE         0x25
#define CAN_WP_FUNCODE_MOTOR_READOHM       0x2c
#define CAN_WP_FUNCODE_MOTOR_SETALARMCODE  0x26
#define CAN_WP_FUNCODE_MOTOR_SETALARMMASK  0x27
#define CAN_WP_FUNCODE_MOTOR_READALARM     0x28
#define CAN_WP_FUNCODE_MOTOR_READ          0x29
#define CAN_WP_FUNCODE_MOTOR_ALARM         0x2b

//SR
#define CAN_WP_FUNCODE_SR_POWERSAVE     0x21
#define CAN_WP_FUNCODE_SR_EN            0x22
#define CAN_WP_FUNCODE_SR_RESET         0x23
#define CAN_WP_FUNCODE_SR_SETDUTY       0x24
#define CAN_WP_FUNCODE_SR_WRITE         0x25
#define CAN_WP_FUNCODE_SR_READOHM       0x2c
#define CAN_WP_FUNCODE_SR_SETALARMCODE  0x26
#define CAN_WP_FUNCODE_SR_SETALARMMASK  0x27
#define CAN_WP_FUNCODE_SR_READALARM     0x28
#define CAN_WP_FUNCODE_SR_READ          0x29
#define CAN_WP_FUNCODE_SR_ALARM         0x2b


typedef struct {
    unsigned int funcode:12;
    unsigned int srcid:8;
    unsigned int desid:8;
    unsigned int flag:1;
    unsigned int candir:1;
    unsigned int xtd:1;
    unsigned int dlc;
    unsigned int data[2];
}CAN_WP;


#define ID_MASK  (unsigned char)0xf0
#define ID_GROUP_BROADCAST(id) (id | ~ID_MASK)
#define ID_ALL_BROADCAST 0xff



#define DEFINE_CAN_WP_FRAME(frame) CAN_WP frame =  \
              {.flag = 1,.candir =0,.xtd=1,.srcid=0x11}


/*static inline unsigned int wpdevtype2index(unsigned char type) {
    return ~(unsigned char)ID_MASK

} */


#endif /*__CAN__WP__H__*/
