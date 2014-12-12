
#include "can_wp.h"
#include <stdbool.h>
#include "pf_can.h"
#include "module.h"
#include "pf_timertick.h"


void (*wphandler[5])(CAN_WP *frame);


bool wpHeartBeat(unsigned char id, unsigned int timeout,atomic *flag) {
    atomicClear(flag);
    DEFINE_CAN_WP_FRAME(frame);
    frame.funcode = CAN_WP_FUNCODE_HEARDBEAT;
    frame.desid = id;
    frame.dlc = 0;
    CANSend_noblock(MODULE_ID_DCAN0, (CAN_FRAME *)&frame);
    withintimedo(tmark, timeout) {
        if (atomicTestClear(flag)) {
            return true;
        }
    }
    return false;
}








