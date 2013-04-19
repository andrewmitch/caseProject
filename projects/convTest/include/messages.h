#ifndef __MESSAGES_H
#define __MESSAGES_H

/********************************************************
* MESSAGES
********************************************************/

typedef enum {
    readyRequestBroadcast = 1,
    startRequestBroadcast,
    stopRequestBroadcast,
    pauseRequestBroadcast,
    resumeRequestBroadcast,
    
    platform1PickupReq,
    robotInPickupAck,
    robotInPickupConfirm,
    platform1PickupConfirmed,
    
    robotInDropReq,
    conveyorDropAck,
    
    conveyorPickupReq,
    robotOutPickupAck,
    robotOutPickupConfirm,
    conveyorPickupConfirmed,
    conveyorPickupFailed,
    
    robotOutDropReq,
    platform2DropAllowed,
    robotOutDropped,
    
    error
} message_t;

#endif
