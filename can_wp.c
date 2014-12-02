
#include "can_wp.h"
#include <stdbool.h>


void (*wphandler[5])(CAN_WP *frame);


bool wpHeartBeat(unsigned char id, unsigned int timeout) {
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_SF_HEARDBEAT;
    frame.desid = SIFU_ID(id);
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(&g_flagSfHeartBeat)) {
            return true;
        }
    }
    return false;
}


bool wpRegestHandler(unsigned int devtype,void (*handlder)(CAN_WP *frame)){

}






