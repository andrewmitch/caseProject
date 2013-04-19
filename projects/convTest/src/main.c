#include <stdbool.h>
#include <ucos_ii.h>
#include <bsp.h>
#include <osutils.h>
#include <leds.h>
#include <buttons.h>
#include <interface.h>
#include <conveyor.h>
#include <can.h>
#include <messages.h>
#include <lcd.h>

/*************************************************************************
*                  PRIORITIES
*************************************************************************/

enum {
  APP_TASK_CAN_READ_PRIO =4,
  APP_TASK_MONITOR_SENS_PRIO,
  APP_TASK_CTRL_CONV_PRIO
};

/*************************************************************************
*                  APPLICATION TASK STACKS
*************************************************************************/

enum {
  APP_TASK_CAN_READ_STK_SIZE =256,
  APP_TASK_MONITOR_SENS_STK_SIZE = 256,
  APP_TASK_CTRL_CONV_STK_SIZE = 256
};
static OS_STK appTaskCanReadStk[APP_TASK_CAN_READ_STK_SIZE];
static OS_STK appTaskMonitorSensStk[APP_TASK_MONITOR_SENS_STK_SIZE];
static OS_STK appTaskCtrlConvStk[APP_TASK_CTRL_CONV_STK_SIZE];

/*************************************************************************
*                  APPLICATION FUNCTION PROTOTYPES
*************************************************************************/

static void appTaskMonitorSens(void *pdata);
static void appTaskCtrlConv(void *pdata);
static void appTaskCanRead(void *pdata);

void canHandler(void);
void sendMessage(message_t);

               

// Other Stuff
canMessage_t can1RxBuffer;
bool can1RxDone;
bool ack = false;



/*************************************************************************
*                    GLOBAL FUNCTION DEFINITIONS
*************************************************************************/

int main() {
  /* Initialise the hardware */
  bspInit();
  conveyorInit();
 
  canRxInterrupt(canHandler);
  
  /* Initialise the OS */
  OSInit();                                                   

  /* Create Tasks */
  
  OSTaskCreate(appTaskCanRead,                               
               (void *)0,
               (OS_STK *)&appTaskCanReadStk[APP_TASK_CAN_READ_STK_SIZE - 1],
               APP_TASK_CAN_READ_PRIO);
  
  OSTaskCreate(appTaskMonitorSens,                               
               (void *)0,
               (OS_STK *)&appTaskMonitorSensStk[APP_TASK_MONITOR_SENS_STK_SIZE - 1],
               APP_TASK_MONITOR_SENS_PRIO);
   
  OSTaskCreate(appTaskCtrlConv,                               
               (void *)0,
               (OS_STK *)&appTaskCtrlConvStk[APP_TASK_CTRL_CONV_STK_SIZE - 1],
               APP_TASK_CTRL_CONV_PRIO);
   
  /* Start the OS */
  OSStart();                                                  
  
  /* Should never arrive here */ 
  return 0;      
}

/*************************************************************************
*                   APPLICATION TASK DEFINITIONS
*************************************************************************/

static void appTaskMonitorSens(void *pdata) {
  
  
  while (true) {
    interfaceLedSetState(D1_LED | D2_LED | D3_LED | D4_LED, LED_OFF);
    ledSetState(USB_LINK_LED, LED_OFF);
    ledSetState(USB_CONNECT_LED, LED_OFF);
    
    if (conveyorItemPresent(CONVEYOR_SENSOR_1)) {
        interfaceLedSetState(D1_LED, LED_ON);
        ledSetState(USB_LINK_LED, LED_ON);
    } 
    if (conveyorItemPresent(CONVEYOR_SENSOR_2)) {
        interfaceLedSetState(D2_LED, LED_ON);
        ledSetState(USB_CONNECT_LED, LED_ON);
    } 
    
    OSTimeDly(20);
  }
}

static void appTaskCtrlConv(void *pdata) {
  while (true) {
      if(conveyorItemPresent(CONVEYOR_SENSOR_2) && (!conveyorItemPresent(CONVEYOR_SENSOR_1))) {
          conveyorSetState(CONVEYOR_REVERSE);
          OSTimeDly(250);
          }
      if(conveyorItemPresent(CONVEYOR_SENSOR_1) && (!conveyorItemPresent(CONVEYOR_SENSOR_2))) {
         conveyorSetState(CONVEYOR_OFF);
         if (!ack) {
          sendMessage(conveyorPickupReq);
         }
         OSTimeDly(250);
         }
    }
}

void canHandler(void) {
  if (canReady(1)) {
    canRead(1, &can1RxBuffer);
    can1RxDone = true;
  }
}

void sendMessage(message_t message) {
    canMessage_t txMsg = {0x2, 4, message, 0}; // ID, Length, Data A, Data B
    canWrite(1, &txMsg);
}

static void appTaskCanRead(void *pdata) {
  osStartTick();
  canRxInterrupt(canHandler);
  
  while(true) {
    if (can1RxDone) { 
      can1RxDone = false;
      
      lcdSetTextPos(2,2);
      lcdWrite("%d", can1RxBuffer.dataA);
      
    switch (can1RxBuffer.dataA) {
      case robotOutPickupAck:
        ack = true;
        break;
      case robotOutPickupConfirm:
        sendMessage(conveyorPickupConfirmed);
        break;
      }
    }
    OSTimeDly(100);
    
  }
}