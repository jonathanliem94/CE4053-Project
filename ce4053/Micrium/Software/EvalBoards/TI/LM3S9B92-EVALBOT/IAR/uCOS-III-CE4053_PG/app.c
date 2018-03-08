/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2009-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : FUZZI
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>
#include <stdio.h>
#include "driverlib/timer.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include <os.h>

/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define ONESECONDTICK             7000000

#define TASK1PERIOD                   10
#define TASK2PERIOD                   20


#define WORKLOAD1                     2
#define WORKLOAD2                     2

#define LED_BLINK_Proc                 0x01
#define MOVE_FOR_Proc                   0x02
#define MOVE_BACK_Proc                  0x04
#define TURN_LEFT_Proc                  0x08
#define TURN_RIGHT_Proc                 0x10


// how many clock cycles per OSTickCtr
#define TIMERDIV                      (BSP_CPUClkFreq() / (CPU_INT32U)OSCfg_TickRate_Hz)




/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB       AppTaskStartTCB;
static  CPU_STK      AppTaskStartStk[APP_TASK_START_STK_SIZE];

static  OS_TCB       LEDBlinkTCB;
static  CPU_STK      LEDBlinkStk[LED_BLINK_STK_SIZE];

static  OS_TCB       moveForwardTCB;
static  CPU_STK      moveForwardStk[MOV_FORWARD_STK_SIZE];

static  OS_TCB       moveBackwardTCB;
static  CPU_STK      moveBackwardStk[MOV_BACKWARD_STK_SIZE];

static  OS_TCB       leftTurnTCB;
static  CPU_STK      leftTurnStk[LEFT_TURN_STK_SIZE];

static  OS_TCB       rightTurnTCB;
static  CPU_STK      rightTurnStk[RIGHT_TURN_STK_SIZE];

CPU_INT32U      iCnt = 0;
CPU_INT08U      Left_tgt;
CPU_INT08U      Right_tgt;
CPU_INT32U      iToken  = 0;
CPU_INT32U      iCounter= 1;
CPU_INT32U      iMove   = 10;
CPU_INT32U      measure=0;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void        AppRobotMotorDriveSensorEnable    ();
void        IntWheelSensor                    ();
void        RoboTurn                          (tSide dir, CPU_INT16U seg, CPU_INT16U speed);

static  void        AppTaskStart                 (void  *p_arg);
static  void        LEDBlink                   (void  *p_arg);
static  void        moveForward                   (void  *p_arg);
static  void        moveBackward                   (void  *p_arg);
static  void        leftTurn                   (void  *p_arg);
static  void        rightTurn                   (void  *p_arg);

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int  main (void)
{
  OS_ERR  err;
  
  BSP_IntDisAll();                                            /* Disable all interrupts.                              */
  OSInit(&err);                                               /* Init uC/OS-III.                                      */
  
  
  OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,           /* Create the start task                                */
               (CPU_CHAR   *)"App Task Start",
               (OS_TASK_PTR ) AppTaskStart,
               (void       *) 0,
               (OS_PRIO     ) APP_TASK_START_PRIO,
               (OS_PERIOD   ) 0u,
               (CPU_STK    *)&AppTaskStartStk[0],
               (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10u,
               (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,
               (OS_MSG_QTY  ) 0u,
               (OS_TICK     ) 0u,
               (void       *) (CPU_INT32U) 0, 
               (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
               (OS_ERR     *)&err);
  
  OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void  *p_arg)
{
  CPU_INT32U  clk_freq;
  CPU_INT32U  cnts;
  OS_ERR      err;
  (void)&p_arg;
  BSP_Init();                                                 /* Initialize BSP functions                             */
  CPU_Init();                                                 /* Initialize the uC/CPU services                       */
  clk_freq = BSP_CPUClkFreq();                                /* Determine SysTick reference freq.                    */
  cnts     = clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;        /* Determine nbr SysTick increments                     */
  OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */
  CPU_TS_TmrFreqSet(clk_freq);
  
  /* Enable Wheel ISR Interrupt */
  AppRobotMotorDriveSensorEnable();
  
  /* Initialise the 2 Main Tasks to  Deleted State */
//////////////////////////////////      create LED Blink Task   //////////////////////////////////////////
  OSRecTaskCreate((OS_TCB     *)&LEDBlinkTCB, 
               (CPU_CHAR   *)"LED Blink", 
               (OS_TASK_PTR ) LEDBlink, 
               (void       *) 0, 
               (OS_PRIO     ) LED_BLINK_PRIO, 
               (OS_PERIOD   ) 5000u,
               (CPU_STK    *)&LEDBlinkStk[0], 
               (CPU_STK_SIZE) LED_BLINK_STK_SIZE / 10u, 
               (CPU_STK_SIZE) LED_BLINK_STK_SIZE, 
               (OS_MSG_QTY  ) 0u, 
               (OS_TICK     ) 0u, 
               (void       *)(CPU_INT32U) 1, 
               (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), 
               (OS_ERR     *)&err);
//////////////////////////////////      create Move Forward Task   //////////////////////////////////////////  
//  OSRecTaskCreate((OS_TCB     *)&moveForwardTCB, 
//               (CPU_CHAR   *)"Move Forwards", 
//               (OS_TASK_PTR ) moveForward, 
//               (void       *) 0, 
//               (OS_PRIO     ) MOV_FORWARD_PRIO,  
//               (OS_PERIOD   ) 10u,
//               (CPU_STK    *)&moveForwardStk[0], 
//               (CPU_STK_SIZE) MOV_FORWARD_STK_SIZE / 10u, 
//               (CPU_STK_SIZE) MOV_FORWARD_STK_SIZE, 
//               (OS_MSG_QTY  ) 0u, 
//               (OS_TICK     ) 0u, 
//               (void       *) (CPU_INT32U) 2, 
//               (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), 
//               (OS_ERR     *)&err);
  
//////////////////////////////////      create Move Backward Task   //////////////////////////////////////////  
//  OSRecTaskCreate((OS_TCB     *)&moveBackwardTCB, 
//               (CPU_CHAR   *)"Move Backwards", 
//               (OS_TASK_PTR ) moveBackward, 
//               (void       *) 0, 
//               (OS_PRIO     ) MOV_BACKWARD_PRIO,  
//               (OS_PERIOD   ) 17u,
//               (CPU_STK    *)&moveBackwardStk[0], 
//               (CPU_STK_SIZE) MOV_BACKWARD_STK_SIZE / 10u, 
//               (CPU_STK_SIZE) MOV_BACKWARD_STK_SIZE, 
//               (OS_MSG_QTY  ) 0u, 
//               (OS_TICK     ) 0u, 
//               (void       *) (CPU_INT32U) 3, 
//               (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), 
//               (OS_ERR     *)&err);

//////////////////////////////////      create Left Turn Task   //////////////////////////////////////////  
//  OSRecTaskCreate((OS_TCB     *)&leftTurnTCB, 
//               (CPU_CHAR   *)"Left Turn", 
//               (OS_TASK_PTR ) leftTurn, 
//               (void       *) 0, 
//               (OS_PRIO     ) LEFT_TURN_PRIO,  
//               (OS_PERIOD   ) 25u,
//               (CPU_STK    *)&leftTurnStk[0], 
//               (CPU_STK_SIZE) LEFT_TURN_STK_SIZE / 10u, 
//               (CPU_STK_SIZE) LEFT_TURN_STK_SIZE, 
//               (OS_MSG_QTY  ) 0u, 
//               (OS_TICK     ) 0u, 
//               (void       *) (CPU_INT32U) 4, 
//               (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), 
//               (OS_ERR     *)&err);

//////////////////////////////////      create Right Turn Task   //////////////////////////////////////////  
//  OSRecTaskCreate((OS_TCB     *)&rightTurnTCB, 
//               (CPU_CHAR   *)"Right Turn", 
//               (OS_TASK_PTR ) rightTurn, 
//               (void       *) 0, 
//               (OS_PRIO     ) RIGHT_TURN_PRIO,  
//               (OS_PERIOD   ) 47u,
//               (CPU_STK    *)&rightTurnStk[0], 
//               (CPU_STK_SIZE) RIGHT_TURN_STK_SIZE / 10u, 
//               (CPU_STK_SIZE) RIGHT_TURN_STK_SIZE, 
//               (OS_MSG_QTY  ) 0u, 
//               (OS_TICK     ) 0u, 
//               (void       *) (CPU_INT32U) 5, 
//               (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), 
//               (OS_ERR     *)&err);
  
  /* Delete this task */
  OSTaskDel((OS_TCB *)0, &err); //      this deletes the AppTaskStart, leaving the above 5 tasks in the queue
  
}

static  void  LEDBlink (void  *p_arg)
{   
  OS_ERR      err;
  CPU_INT32U  k, i, j;
  BSP_LED_Off(0u);      // turn led off
  //    code below is to blink five times with half a second interval betw blinks
  for(k=0; k<10; k++)   
  {
    BSP_LED_Toggle(0u);
    for(i=0; i <ONESECONDTICK/4; i++)
      j = ((i * 2)+j);
  }
  
  BSP_LED_Off(0u);
  //    turn off again
/* Delete this task */
  OSRecTaskDelete((OS_TCB *)0, &err);
            
}

static  void  moveForward (void  *p_arg)
{ 
  OS_ERR      err;
  CPU_INT32U  k, i, j;
  
  RoboTurn(FRONT, 16, 50);
  for(k=0; k<3; k++)        //      for WORKLOAD seconds
  {
    for(i=0; i <ONESECONDTICK; i++){
      j=2*i;
    }
  }
/* Delete this task */
  OSRecTaskDelete((OS_TCB *)0, &err);
}


static  void moveBackward (void *p_arg)
{
  OS_ERR      err;
  CPU_INT32U  k, i, j;
  
  RoboTurn(BACK, 16, 50);
  for(k=0; k<3; k++)        //      for WORKLOAD seconds
  {
    for(i=0; i <ONESECONDTICK; i++){
      j=2*i;
    }
  }
/* Delete this task */
  OSRecTaskDelete((OS_TCB *)0, &err);
  
}

static  void leftTurn (void *p_arg)
{
  OS_ERR      err;
  CPU_INT32U  k, i, j;
  
  RoboTurn(LEFT_SIDE, 16, 50);
  for(k=0; k<3; k++)        //      for WORKLOAD seconds
  {
    for(i=0; i <ONESECONDTICK; i++){
      j=2*i;
    }
  }
/* Delete this task */
  OSRecTaskDelete((OS_TCB *)0, &err);
  
}

static  void rightTurn (void *p_arg)
{
  OS_ERR      err;
  CPU_INT32U  k, i, j;
  
  RoboTurn(RIGHT_SIDE, 16, 50);
  for(k=0; k<3; k++)        //      for WORKLOAD seconds
  {
    for(i=0; i <ONESECONDTICK; i++){
      j=2*i;
    }
  }
/* Delete this task */
  OSRecTaskDelete((OS_TCB *)0, &err);
  
}

static  void  AppRobotMotorDriveSensorEnable ()
{
  BSP_WheelSensorEnable();
  BSP_WheelSensorIntEnable(RIGHT_SIDE, SENSOR_A, (CPU_FNCT_VOID)IntWheelSensor);
  BSP_WheelSensorIntEnable(LEFT_SIDE, SENSOR_A, (CPU_FNCT_VOID)IntWheelSensor);
}


void IntWheelSensor()
{
  CPU_INT32U         ulStatusR_A;
  CPU_INT32U         ulStatusL_A;
  
  static CPU_INT08U CountL = 0;
  static CPU_INT08U CountR = 0;
  
  static CPU_INT08U data = 0;
  
  ulStatusR_A = GPIOPinIntStatus(RIGHT_IR_SENSOR_A_PORT, DEF_TRUE);
  ulStatusL_A = GPIOPinIntStatus(LEFT_IR_SENSOR_A_PORT, DEF_TRUE);
  
  if (ulStatusR_A & RIGHT_IR_SENSOR_A_PIN)
  {
    GPIOPinIntClear(RIGHT_IR_SENSOR_A_PORT, RIGHT_IR_SENSOR_A_PIN);           /* Clear interrupt.*/
    CountR = CountR + 1;
  }
  
  if (ulStatusL_A & LEFT_IR_SENSOR_A_PIN)
  {
    GPIOPinIntClear(LEFT_IR_SENSOR_A_PORT, LEFT_IR_SENSOR_A_PIN);
    CountL = CountL + 1;
  }
  
  if((CountL >= Left_tgt) && (CountR >= Right_tgt))
  {
    data = 0x11;
    Left_tgt = 0;
    Right_tgt = 0;
    CountL = 0;
    CountR = 0;
    BSP_MotorStop(LEFT_SIDE);
    BSP_MotorStop(RIGHT_SIDE);
  }
  else if(CountL >= Left_tgt)
  {
    data = 0x10;
    Left_tgt = 0;
    CountL = 0;
    BSP_MotorStop(LEFT_SIDE);
  }
  else if(CountR >= Right_tgt)
  {
    data = 0x01;
    Right_tgt = 0;
    CountR = 0;
    BSP_MotorStop(RIGHT_SIDE);
  }
  return;
}

void RoboTurn(tSide dir, CPU_INT16U seg, CPU_INT16U speed)
{
  Left_tgt = seg;
  Right_tgt = seg;
  
  BSP_MotorStop(LEFT_SIDE);
  BSP_MotorStop(RIGHT_SIDE);
  
  BSP_MotorSpeed(LEFT_SIDE, speed <<8u);
  BSP_MotorSpeed(RIGHT_SIDE,speed <<8u);
  
  switch(dir)
  {
  case FRONT :
    BSP_MotorDir(RIGHT_SIDE,FORWARD);
    BSP_MotorDir(LEFT_SIDE,FORWARD);
    BSP_MotorRun(LEFT_SIDE);
    BSP_MotorRun(RIGHT_SIDE);
    break;
    
  case BACK :
    BSP_MotorDir(LEFT_SIDE,REVERSE);
    BSP_MotorDir(RIGHT_SIDE,REVERSE);
    BSP_MotorRun(RIGHT_SIDE);
    BSP_MotorRun(LEFT_SIDE);
    break;
    
  case LEFT_SIDE :
    BSP_MotorDir(RIGHT_SIDE,FORWARD);
    BSP_MotorDir(LEFT_SIDE,REVERSE);
    BSP_MotorRun(LEFT_SIDE);
    BSP_MotorRun(RIGHT_SIDE);
    break;
    
  case RIGHT_SIDE:
    BSP_MotorDir(LEFT_SIDE,FORWARD);
    BSP_MotorDir(RIGHT_SIDE,REVERSE);
    BSP_MotorRun(RIGHT_SIDE);
    BSP_MotorRun(LEFT_SIDE);
    break;
    
  default:
    BSP_MotorStop(LEFT_SIDE);
    BSP_MotorStop(RIGHT_SIDE);
    break;
  }
  
  return;
}
