/*
***********************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2011; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*                                                   TICK MANAGEMENT
*
* File    : OS_TICK.C
* By      : JJL
* Version : V3.02.00
*
* LICENSING TERMS:
* ---------------
*           uC/OS-III is provided in source form for FREE short-term evaluation, for educational use or 
*           for peaceful research.  If you plan or intend to use uC/OS-III in a commercial application/
*           product then, you need to contact Micrium to properly license uC/OS-III for its use in your 
*           application/product.   We provide ALL the source code for your convenience and to help you 
*           experience uC/OS-III.  The fact that the source is provided does NOT mean that you can use 
*           it commercially without paying a licensing fee.
*
*           Knowledge of the source code may NOT be used to develop a similar product.
*
*           Please help us continue to provide the embedded community with the finest software available.
*           Your honesty is greatly appreciated.
*
*           You can contact us at www.micrium.com, or by phone at +1 (954) 217-2036.
************************************************************************************************************************
*/

#include <os.h>
#include <heap.h>
#include  <avltree.h>
#include <stack.h>
extern struct avl_tree OS_AVL_TREE;
extern struct stack_node* OS_MUTEX_STACK_HEAD;
extern struct heap OS_REC_HEAP;
struct node new_nodeArr[100];
struct os_avl_node new_avl_nodeArr[200];
CPU_INT16U avl_count=0;;
CPU_INT16U count = 0;
#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *os_tick__c = "$Id: $";
#endif
/*
************************************************************************************************************************
*                                                  LOCAL PROTOTYPES
************************************************************************************************************************
*/

/*
************************************************************************************************************************
*                                                 Reviving a Recursive Task
  This will run everytime OS_TickTask is run.
  When TickTask is run, OSTickCtr will be incremented, this is a value we will compare against the 
    deadline of the recursive tasks inside heap.
  We always compare against the root node of the heap since it will always be the one with the lowest deadline.
  When that condition is ture, we "create" the task again by making p_tcb = to the one stored in the heap
  But we add additional stuff, and also update the deadline to period+OSTickCtr
      insert the prio and the task into ready list
  after adding all these, we need to remove the task from the heap, and add it back in since it's deadline will be updated
  This is done by doing the heap and array thing again, followed by pushing and popping of the heap


************************************************************************************************************************
*/


void OS_revive_rec_task(void)		//      insert tasks into ready list
{		

  OS_TCB *p_tcb;	
  OS_TCB *temp_tcb;
  OS_ERR *err;
  struct avl_node* node_for_insertion;
  struct avl_node  *cur;
  struct os_avl_node* node, query;
//  OS_FLAG_GRP syncFlag;
//  OSFlagCreate((OS_FLAG_GRP *)&MyEventFlag,
//             (CPU_CHAR *)"MyEventFlag",
//             (OS_FLAGS)0,
//             (OS_ERR *)&err);
/*
************************************************************************************************************************
*                                                 Adding Recursive Task into ready list
  This will run a pseudo OSTaskCreate to add the tasks into ready list at the start once they are all in the heap, pushed by OSRecTaskCreate()
  We iterate through the heap to place each node (p_tcb) into the readylist

  Only done at start --> OSTickCtr == 0
************************************************************************************************************************
*/
//  if (OSTickCtr == 5) {
  if(syncRelease == 0)      {
//    if(&MyEventFlag.Flags == 1) {
  //  iterate through heap (size 5) 
  //  add them all into readylist at one go
    for (int k = 0; k < 3; k++) 
    {
      p_tcb = OS_REC_HEAP.node_arr[k]->p_tcb;
      
      CPU_SR_ALLOC(); 		      
      /*insert into AVL tree as well */
      //    &node = (struct os_avl_node *)realloc(sizeof(struct os_avl_node));
      //      //##########################      INCREMENT DEADLINE TO DEAL WITH SAME DEADLINES  #########################################################################
      //      for (int p = 0; p < k; p++)
      //      {
      //        //      check if any existing p_tcb in heap first
      //        if (OS_REC_HEAP.node_arr[p]->p_tcb != 0)
      //        {
      //          //      iterate through heap check if any same deadline
      //           if (OS_REC_HEAP.node_arr[p]->p_tcb->Deadline == OS_REC_HEAP.node_arr[k]->p_tcb->Deadline)
      //             OS_REC_HEAP.node_arr[k]->p_tcb->Deadline += 1;     //      if yes, change the deadline by adding 1
      //             OS_REC_HEAP.node_arr[k]->deadline =  OS_REC_HEAP.node_arr[k]->p_tcb->Deadline;
      //        }
      //      }//#######################################################################################################################################################
      new_avl_nodeArr[avl_count].deadline = p_tcb->Deadline;
      new_avl_nodeArr[avl_count].p_tcb1 = p_tcb;
      
      // if inserting a dupe, the function will return the node with the same key, but no insertion done
      // if inserting a non-dupe, the function inserts normally and returns the node that was inserted
      node_for_insertion = avl_insert(&OS_AVL_TREE, &new_avl_nodeArr[avl_count].avl, cmp_func);
      node_for_insertion->tcb_count++;
      if (&new_avl_nodeArr[avl_count].avl != node_for_insertion)
      {
        node = _get_entry(node_for_insertion, struct os_avl_node, avl);
        if (node->p_tcb1 == 0) node->p_tcb1 = new_avl_nodeArr[avl_count].p_tcb1;
        else if (node->p_tcb2 == 0) node->p_tcb2 = new_avl_nodeArr[avl_count].p_tcb1;
        else if (node->p_tcb3 == 0) node->p_tcb3 = new_avl_nodeArr[avl_count].p_tcb1;
      }
      
      avl_count++;
      if (avl_count == 200)
        avl_count=0;
      //      OS_CRITICAL_ENTER();		        //      will break robot if enabled
      OS_PrioInsert(p_tcb->Prio);		
      OS_RdyListInsertTail(p_tcb);
      
      
#if OS_CFG_DBG_EN > 0u		
      OS_TaskDbgListAdd(p_tcb);		
#endif		
      OSTaskQty++;                                            		
      if (OSRunning != OS_STATE_OS_RUNNING) {                 		
        OS_CRITICAL_EXIT();		
        return;		
      }	
      OS_CRITICAL_EXIT_NO_SCHED();
    }
    syncRelease = 1;    //    sync release flag set to 1;
//    OSFlagPost ((OS_FLAG_GRP *)&MyEventFlag,
//                (OS_FLAGS)0x01,
//                (OS_OPT) OS_OPT_POST_FLAG_CLR,
//                (OS_ERR *)&err);
  
}
//******************************        if no longer at the start       *****************************************************************************************
//******************************   
//******************************   
//******************************   
//******************************   
//******************************   
  
  else {
    while (OSTickCtr == OS_REC_HEAP.node_arr[0]->deadline)     
    {		
      p_tcb = OS_REC_HEAP.node_arr[0]->p_tcb;
      CPU_STK_SIZE   j; 		
      
//      ######################################################################### 
/*      following code checks if the next task to run already exists inside avl,
          if yes, we need to remove from avl and readylist
          this is because it has already missed its deadline in the previous incarnation
*/      


//      query.deadline=p_tcb->Deadline;
//      cur = avl_search(&OS_AVL_TREE, &query.avl, cmp_func);
//      node = _get_entry(cur, struct os_avl_node, avl);
//      if (node->p_tcb1 == p_tcb) 
//      {
//        while ((&OS_MUTEX_STACK_HEAD != 0)&&(OS_MUTEX_STACK_HEAD->data->OwnerTCBPtr == p_tcb))
//        {
//          OS_MUTEX_STACK_HEAD->data->OwnerNestingCtr--;
//          OSMutexPost((OS_MUTEX *)OS_MUTEX_STACK_HEAD->data, (OS_OPT )OS_OPT_POST_NO_SCHED, (OS_ERR *)&err);
//        }
//        OS_RdyListRemove(node->p_tcb1);
//        node->p_tcb1 = 0;
//        avl_remove(&OS_AVL_TREE, cur);
//      }
//      else if (node->p_tcb2 == p_tcb)
//      {
//        while ((&OS_MUTEX_STACK_HEAD != 0)&&(OS_MUTEX_STACK_HEAD->data->OwnerTCBPtr == p_tcb))
//        {
//          OS_MUTEX_STACK_HEAD->data->OwnerNestingCtr--;
//          OSMutexPost((OS_MUTEX *)OS_MUTEX_STACK_HEAD->data, (OS_OPT )OS_OPT_POST_NO_SCHED, (OS_ERR *)&err);
//        }
//        OS_RdyListRemove(node->p_tcb2);
//        node->p_tcb2 = 0;
//        avl_remove(&OS_AVL_TREE, cur);
//      }
//      else if (node->p_tcb3 == p_tcb)
//      {
//        while ((&OS_MUTEX_STACK_HEAD != 0)&&(OS_MUTEX_STACK_HEAD->data->OwnerTCBPtr == p_tcb))
//        {
//          OS_MUTEX_STACK_HEAD->data->OwnerNestingCtr--;
//          OSMutexPost((OS_MUTEX *)OS_MUTEX_STACK_HEAD->data, (OS_OPT )OS_OPT_POST_NO_SCHED, (OS_ERR *)&err);
//        }
//        OS_RdyListRemove(node->p_tcb3);
//        node->p_tcb3 = 0;
//        avl_remove(&OS_AVL_TREE, cur);
//      }
      //      ###########################################################################
      //      #########################################################################
//      query.deadline=p_tcb->Deadline;
//      cur = avl_search(&OS_AVL_TREE, &query.avl, cmp_func);
//      node = _get_entry(cur, struct os_avl_node, avl);
//      
//      if (node->p_tcb1 == p_tcb) 
//      {
//        while ((&OS_MUTEX_STACK_HEAD != 0)&&(OS_MUTEX_STACK_HEAD->data->OwnerTCBPtr == p_tcb))
//        {
//          OSMutexPost((OS_MUTEX *)OS_MUTEX_STACK_HEAD->data, (OS_OPT )OS_OPT_POST_NO_SCHED, (OS_ERR *)&err);
//        }
//        OSRecTaskDelete((OS_TCB *)node->p_tcb1, &err);
//      }
//      else if (node->p_tcb2 == p_tcb)
//      {
//        while ((&OS_MUTEX_STACK_HEAD != 0)&&(OS_MUTEX_STACK_HEAD->data->OwnerTCBPtr == p_tcb))
//        {
//          OSMutexPost((OS_MUTEX *)OS_MUTEX_STACK_HEAD->data, (OS_OPT )OS_OPT_POST_NO_SCHED, (OS_ERR *)&err);
//        }
//        OSRecTaskDelete((OS_TCB *)node->p_tcb2, &err);
//      }
//      else if (node->p_tcb3 == p_tcb)
//      {
//        while ((&OS_MUTEX_STACK_HEAD != 0)&&(OS_MUTEX_STACK_HEAD->data->OwnerTCBPtr == p_tcb))
//        {
//          OSMutexPost((OS_MUTEX *)OS_MUTEX_STACK_HEAD->data, (OS_OPT )OS_OPT_POST_NO_SCHED, (OS_ERR *)&err);
//        }
//        OSRecTaskDelete((OS_TCB *)node->p_tcb3, &err);
//      }
      
      
#if OS_CFG_TASK_REG_TBL_SIZE > 0u 		
      OS_OBJ_QTY     reg_nbr; 		
#endif 		
      CPU_STK       *p_sp; 		
      CPU_SR_ALLOC(); 		
      
      //  *p_err = OS_ERR_NONE; 		
#if 1                                                          /* --------------- CLEAR THE TASK'S STACK --------------- */ 		
      p_sp = p_tcb->StkBasePtr; 		
      for (j = 0u; j < p_tcb->StkSize; j++) {                 /* Stack grows from HIGH to LOW memory                    */ 		
        *p_sp = (CPU_STK)0;                                   /* Clear from bottom of stack and up!                     */ 		
        p_sp++; 		
      } 		
      /* ------- INITIALIZE THE STACK FRAME OF THE TASK ------- */ 		
      p_sp = OSTaskStkInit(p_tcb->TaskEntryAddr, 		
                           p_tcb->TaskEntryArg, 		
                           p_tcb->StkBasePtr, 		
                           p_tcb->StkLimitPtr, 		
                           p_tcb->StkSize, 		
                           p_tcb->Opt); 		
      p_tcb->StkPtr        = p_sp;                            /* Save the new top-of-stack pointer                      */ 	
      p_tcb->Deadline      =    p_tcb->Period+OSTickCtr;        //      need to update the corresponding deadline for each recursive task
      
#if 0 		
      /* -------------- INITIALIZE THE TCB FIELDS ------------- */ 		
      p_tcb->TaskEntryAddr = p_task;                          /* Save task entry point address                          */ 		
      p_tcb->TaskEntryArg  = p_arg;                           /* Save task entry argument                               */ 		
      
      p_tcb->NamePtr       = p_name;                          /* Save task name                                         */ 		
      
      p_tcb->Prio          = prio;                            /* Save the task's priority                               */ 		
      p_tcb->Period        = period;
      p_tcb->Deadline      = period+OSTickCtr;
      
      p_tcb->StkLimitPtr   = p_stk_limit;                     /* Save the stack limit pointer                           */ 		
      
      p_tcb->TimeQuanta    = time_quanta;                     /* Save the #ticks for time slice (0 means not sliced)    */ 	
      
      
#endif 		
#endif 		
      p_tcb->TaskState  = (OS_STATE)OS_TASK_STATE_RDY;  //      assign task state to be ready
      p_tcb->TimeQuanta    = 0;   		
      
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u 		
      if (p_tcb->TimeQuanta == (OS_TICK)0) { 		
        p_tcb->TimeQuantaCtr = OSSchedRoundRobinDfltTimeQuanta; 		
      } else { 		
        p_tcb->TimeQuantaCtr = p_tcb->TimeQuanta; 		
      } 		
#endif 		
#if 0 		
      p_tcb->ExtPtr        = p_ext;                           /* Save pointer to TCB extension                          */ 		
      p_tcb->StkBasePtr    = p_stk_base;                      /* Save pointer to the base address of the stack          */ 		
      p_tcb->StkSize       = stk_size;                        /* Save the stack size (in number of CPU_STK elements)    */ 		
      p_tcb->Opt           = opt;                             /* Save task options                                      */ 		
#endif 		
      
#if OS_CFG_TASK_REG_TBL_SIZE > 0u 		
      for (reg_nbr = 0u; reg_nbr < OS_CFG_TASK_REG_TBL_SIZE; reg_nbr++) { 		
        p_tcb->RegTbl[reg_nbr] = (OS_REG)0; 		
      } 		
#endif 		
#if 0 		
#if OS_CFG_TASK_Q_EN > 0u 		
      OS_MsgQInit(&p_tcb->MsgQ,                               /* Initialize the task's message queue                    */ 		
                  q_size); 		
#endif 		
#endif 		
      
      //OSTaskCreateHook(p_tcb);                              		
      // OS_CRITICAL_ENTER();		//      will break robot if enabled
      
      OS_PrioInsert(p_tcb->Prio);		
      OS_RdyListInsertTail(p_tcb);		
      
      /*insert into AVL tree as well */
//      node = (struct os_avl_node *)malloc(sizeof(struct os_avl_node));
      
#if OS_CFG_DBG_EN > 0u		
      OS_TaskDbgListAdd(p_tcb);		
#endif		
      OSTaskQty++;                                            		
      if (OSRunning != OS_STATE_OS_RUNNING) {                 		
        OS_CRITICAL_EXIT();		
        return;		
      }		
      count=count%100;
      new_nodeArr[count].deadline = p_tcb->Deadline;
      new_nodeArr[count].period = p_tcb->Period;
      new_nodeArr[count].p_tcb = p_tcb;
      new_avl_nodeArr[avl_count].deadline = p_tcb->Deadline;
      new_avl_nodeArr[avl_count].p_tcb1 = p_tcb;
     
      
      OS_CRITICAL_ENTER();    //      do we need this?
      
      // if inserting a dupe, the function will return the node with the same key, but no insertion done
      // if inserting a non-dupe, the function inserts normally and returns the node that was inserted
      node_for_insertion = avl_insert(&OS_AVL_TREE, &new_avl_nodeArr[avl_count].avl, cmp_func);
      node_for_insertion->tcb_count++;
      if (&new_avl_nodeArr[avl_count].avl != node_for_insertion)
      {
        node = _get_entry(node_for_insertion, struct os_avl_node, avl);
        if (node->p_tcb1 == 0) node->p_tcb1 = new_avl_nodeArr[avl_count].p_tcb1;
        else if (node->p_tcb2 == 0) node->p_tcb2 = new_avl_nodeArr[avl_count].p_tcb1;
        else if (node->p_tcb3 == 0) node->p_tcb3 = new_avl_nodeArr[avl_count].p_tcb1;
      }
      
      heap_pop(&OS_REC_HEAP);
      
      OS_CRITICAL_EXIT_NO_SCHED();	
      
      avl_count++;
      if (avl_count == 200)
        avl_count=0;
      heap_push(&OS_REC_HEAP,&new_nodeArr[count]);
      //      
      //      int new_priority = ((p_tcb->Deadline-OSTickCtr)/1000)+3;
      //        OSTaskChangePrio ((OS_TCB   *)p_tcb,
      //                          (OS_PRIO   )new_priority,
      //                          (OS_ERR   *)p_err);
      
      count++;
    }
  }   //      comment this to prove readylist function (syncrhonous release)
  //                P = P;
  //              ts_end1 = CPU_TS_Get32();
  //              P = P;;
  OSSched();	
}		


/*
************************************************************************************************************************
*                                                      TICK TASK
*
* Description: This task is internal to uC/OS-III and is triggered by the tick interrupt.
*
* Arguments  : p_arg     is an argument passed to the task when the task is created (unused).
*
* Returns    : none
*
* Note(s)    : This function is INTERNAL to uC/OS-III and your application should not call it.
************************************************************************************************************************
*/

void  OS_TickTask (void *p_arg)
{
//  CPU_TS32 ts_start1;
//  CPU_TS32 ts_end1; 
//  int P = 0;
//  
//  
//  ts_start1 = CPU_TS_Get32();
    OS_ERR  err;
    CPU_TS  ts;
    


    p_arg = p_arg;                                          /* Prevent compiler warning                               */

    while (DEF_ON) {
        (void)OSTaskSemPend((OS_TICK  )0,
                            (OS_OPT   )OS_OPT_PEND_BLOCKING,
                            (CPU_TS  *)&ts,
                            (OS_ERR  *)&err);               /* Wait for signal from tick interrupt                    */
//           ts_start = OS_TS_GET();
         if (err == OS_ERR_NONE) {		
            if (OSRunning == OS_STATE_OS_RUNNING) {		
		
//              P = P;
//              ts_end1 = CPU_TS_Get32();
//              P = P;;
		OS_revive_rec_task();  // insertion of the tasks into the ready list
                OS_TickListUpdate();                        /* Update all tasks waiting for time                      */ 
            }
        }
    }

}

/*$PAGE*/
/*
************************************************************************************************************************
*                                                 INITIALIZE TICK TASK
*
* Description: This function is called by OSInit() to create the tick task.
*
* Arguments  : p_err   is a pointer to a variable that will hold the value of an error code:
*
*                          OS_ERR_TICK_STK_INVALID   if the pointer to the tick task stack is a NULL pointer
*                          OS_ERR_TICK_STK_SIZE      indicates that the specified stack size
*                          OS_ERR_PRIO_INVALID       if the priority you specified in the configuration is invalid
*                                                      (There could be only one task at the Idle Task priority)
*                                                      (Maybe the priority you specified is higher than OS_CFG_PRIO_MAX-1
*                          OS_ERR_??                 other error code returned by OSTaskCreate()
*
* Returns    : none
*
* Note(s)    : This function is INTERNAL to uC/OS-III and your application should not call it.
************************************************************************************************************************
*/

void  OS_TickTaskInit (OS_ERR  *p_err)
{
#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

    OSTickCtr         = (OS_TICK)0u;                        /* Clear the tick counter                                 */

    OSTickTaskTimeMax = (CPU_TS)0u;


    OS_TickListInit();                                      /* Initialize the tick list data structures               */

                                                            /* ---------------- CREATE THE TICK TASK ---------------- */
    if (OSCfg_TickTaskPrio >= (OS_CFG_PRIO_MAX - 1u)) {     /* Only one task at the 'Idle Task' priority              */
        *p_err = OS_ERR_PRIO_INVALID;
        return;
    }

    OSTaskCreate((OS_TCB     *)&OSTickTaskTCB,
                 (CPU_CHAR   *)((void *)"uC/OS-III Tick Task"),
                 (OS_TASK_PTR )OS_TickTask,
                 (void       *)0,
                 (OS_PRIO     )OSCfg_TickTaskPrio,
                 (OS_PERIOD   ) 0u,
                 (CPU_STK    *)OSCfg_TickTaskStkBasePtr,
                 (CPU_STK_SIZE)OSCfg_TickTaskStkLimit,
                 (CPU_STK_SIZE)OSCfg_TickTaskStkSize,
                 (OS_MSG_QTY  )0u,
                 (OS_TICK     )0u,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)p_err);
}

/*$PAGE*/
/*
************************************************************************************************************************
*                                               INITIALIZE THE TICK LIST
*
* Description: This function initializes the tick handling data structures of uC/OS-III.
*
* Arguments  : none
*
* Returns    : None
*
* Note(s)    : This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

void  OS_TickListInit (void)
{
    OS_TICK_SPOKE_IX   i;
    OS_TICK_SPOKE     *p_spoke;



    for (i = 0u; i < OSCfg_TickWheelSize; i++) {
        p_spoke                = (OS_TICK_SPOKE *)&OSCfg_TickWheel[i];
        p_spoke->FirstPtr      = (OS_TCB        *)0;
        p_spoke->NbrEntries    = (OS_OBJ_QTY     )0u;
        p_spoke->NbrEntriesMax = (OS_OBJ_QTY     )0u;
    }
}

/*$PAGE*/
/*
************************************************************************************************************************
*                                                ADD TASK TO TICK LIST
*
* Description: This function is called to place a task in a list of task waiting for either time to expire or waiting to
*              timeout on a pend call.
*
* Arguments  : p_tcb          is a pointer to the OS_TCB of the task to add to the tick list
*              -----
*
*              time           represents either the 'match' value of OSTickCtr or a relative time from the current
*                             value of OSTickCtr as specified by the 'opt' argument..
*
*                             relative when 'opt' is set to OS_OPT_TIME_DLY
*                             relative when 'opt' is set to OS_OPT_TIME_TIMEOUT
*                             match    when 'opt' is set to OS_OPT_TIME_MATCH
*                             periodic when 'opt' is set to OS_OPT_TIME_PERIODIC
*
*              opt            is an option specifying how to calculate time.  The valid values are:
*              ---
*                                 OS_OPT_TIME_DLY
*                                 OS_OPT_TIME_TIMEOUT
*                                 OS_OPT_TIME_PERIODIC
*                                 OS_OPT_TIME_MATCH
*
*              p_err          is a pointer to a variable that will contain an error code returned by this function.
*              -----
*                                 OS_ERR_NONE           the call was successful and the time delay was scheduled.
*                                 OS_ERR_TIME_ZERO_DLY  if delay is zero or already occurred.
*
* Returns    : None
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
*
*              2) This function is assumed to be called with interrupts disabled.
************************************************************************************************************************
*/

void  OS_TickListInsert (OS_TCB   *p_tcb,
                         OS_TICK   time,
                         OS_OPT    opt,
                         OS_ERR   *p_err)
{
    OS_TICK            tick_delta;
    OS_TICK            tick_next;
    OS_TICK_SPOKE     *p_spoke;
    OS_TCB            *p_tcb0;
    OS_TCB            *p_tcb1;
    OS_TICK_SPOKE_IX   spoke;



    if (opt == OS_OPT_TIME_MATCH) {                              /* Task time is absolute.                            */
        tick_delta = time - OSTickCtr - 1u;
        if (tick_delta > OS_TICK_TH_RDY) {                       /* If delay already occurred, ...                    */
            p_tcb->TickCtrMatch = (OS_TICK        )0u;
            p_tcb->TickRemain   = (OS_TICK        )0u;
            p_tcb->TickSpokePtr = (OS_TICK_SPOKE *)0;
           *p_err               =  OS_ERR_TIME_ZERO_DLY;         /* ... do NOT delay.                                 */
            return;
        }
        p_tcb->TickCtrMatch = time;
        p_tcb->TickRemain   = tick_delta + 1u;

    } else if (time > (OS_TICK)0u) {
        if (opt == OS_OPT_TIME_PERIODIC) {                       /* Task time is periodic.                            */
            tick_next  = p_tcb->TickCtrPrev + time;
            tick_delta = tick_next - OSTickCtr - 1u;
            if (tick_delta < time) {                             /* If next periodic delay did NOT already occur, ... */
                p_tcb->TickCtrMatch = tick_next;                 /* ... set  next  periodic delay;                ... */
            } else {
                p_tcb->TickCtrMatch = OSTickCtr + time;          /* ... else reset periodic delay.                    */
            }
            p_tcb->TickRemain   = p_tcb->TickCtrMatch - OSTickCtr;
            p_tcb->TickCtrPrev  = p_tcb->TickCtrMatch;

        } else {                                                 /* Task time is relative to current.                 */
            p_tcb->TickCtrMatch = OSTickCtr + time;
            p_tcb->TickRemain   = time;
        }

    } else {                                                     /* Zero time delay; ...                              */
        p_tcb->TickCtrMatch = (OS_TICK        )0u;
        p_tcb->TickRemain   = (OS_TICK        )0u;
        p_tcb->TickSpokePtr = (OS_TICK_SPOKE *)0;
       *p_err               =  OS_ERR_TIME_ZERO_DLY;             /* ... do NOT delay.                                 */
        return;
    }


    spoke   = (OS_TICK_SPOKE_IX)(p_tcb->TickCtrMatch % OSCfg_TickWheelSize);
    p_spoke = &OSCfg_TickWheel[spoke];

    if (p_spoke->NbrEntries == (OS_OBJ_QTY)0u) {                 /* First entry in the spoke                          */
        p_tcb->TickNextPtr   = (OS_TCB   *)0;
        p_tcb->TickPrevPtr   = (OS_TCB   *)0;
        p_spoke->FirstPtr    =  p_tcb;
        p_spoke->NbrEntries  = (OS_OBJ_QTY)1u;
    } else {
        p_tcb1     = p_spoke->FirstPtr;                          /* Point to current first TCB in the list            */
        while (p_tcb1 != (OS_TCB *)0) {
            p_tcb1->TickRemain = p_tcb1->TickCtrMatch            /* Compute time remaining of current TCB in list     */
                               - OSTickCtr;
            if (p_tcb->TickRemain > p_tcb1->TickRemain) {        /* Do we need to insert AFTER current TCB in list?   */
                if (p_tcb1->TickNextPtr != (OS_TCB *)0) {        /* Yes, are we pointing at the last TCB in the list? */
                    p_tcb1               =  p_tcb1->TickNextPtr; /* No,  Point to next TCB in the list                */
                } else {
                    p_tcb->TickNextPtr   = (OS_TCB *)0;
                    p_tcb->TickPrevPtr   =  p_tcb1;
                    p_tcb1->TickNextPtr  =  p_tcb;               /* Yes, TCB to add is now new last entry in the list */
                    p_tcb1               = (OS_TCB *)0;          /* Break loop                                        */
                }
            } else {                                             /* Insert before the current TCB                     */
                if (p_tcb1->TickPrevPtr == (OS_TCB *)0) {        /* Are we inserting before the first TCB?            */
                    p_tcb->TickPrevPtr   = (OS_TCB *)0;
                    p_tcb->TickNextPtr   =  p_tcb1;
                    p_tcb1->TickPrevPtr  =  p_tcb;
                    p_spoke->FirstPtr    =  p_tcb;
                } else {                                         /* Insert in between 2 TCBs already in the list      */
                    p_tcb0               =  p_tcb1->TickPrevPtr;
                    p_tcb->TickPrevPtr   =  p_tcb0;
                    p_tcb->TickNextPtr   =  p_tcb1;
                    p_tcb0->TickNextPtr  =  p_tcb;
                    p_tcb1->TickPrevPtr  =  p_tcb;
                }
                p_tcb1 = (OS_TCB *)0;                            /* Break loop                                        */
            }
        }
        p_spoke->NbrEntries++;
    }
    if (p_spoke->NbrEntriesMax < p_spoke->NbrEntries) {          /* Keep track of maximum # of entries in each spoke  */
        p_spoke->NbrEntriesMax = p_spoke->NbrEntries;
    }
    p_tcb->TickSpokePtr = p_spoke;                               /* Link back to tick spoke                           */
   *p_err               = OS_ERR_NONE;
}

/*$PAGE*/
/*
************************************************************************************************************************
*                                         REMOVE A TASK FROM THE TICK LIST
*
* Description: This function is called to remove a task from the tick list
*
* Arguments  : p_tcb          Is a pointer to the OS_TCB to remove.
*              -----
*
* Returns    : none
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
*
*              2) This function is assumed to be called with interrupts disabled.
************************************************************************************************************************
*/

void  OS_TickListRemove (OS_TCB  *p_tcb)
{
    OS_TICK_SPOKE  *p_spoke;
    OS_TCB         *p_tcb1;
    OS_TCB         *p_tcb2;



    p_spoke = p_tcb->TickSpokePtr;
    if (p_spoke != (OS_TICK_SPOKE *)0) {                              /* Confirm that task is in tick list            */
        p_tcb->TickRemain = (OS_TICK)0u;
        if (p_spoke->FirstPtr == p_tcb) {                             /* Is timer to remove at the beginning of list? */
            p_tcb1            = (OS_TCB *)p_tcb->TickNextPtr;         /* Yes                                          */
            p_spoke->FirstPtr = p_tcb1;
            if (p_tcb1 != (OS_TCB *)0) {
                p_tcb1->TickPrevPtr = (void *)0;
            }
        } else {
            p_tcb1              = p_tcb->TickPrevPtr;                 /* No, remove timer from somewhere in the list  */
            p_tcb2              = p_tcb->TickNextPtr;
            p_tcb1->TickNextPtr = p_tcb2;
            if (p_tcb2 != (OS_TCB *)0) {
                p_tcb2->TickPrevPtr = p_tcb1;
            }
        }
        p_tcb->TickNextPtr  = (OS_TCB        *)0;
        p_tcb->TickPrevPtr  = (OS_TCB        *)0;
        p_tcb->TickSpokePtr = (OS_TICK_SPOKE *)0;
        p_tcb->TickCtrMatch = (OS_TICK        )0u;
        p_spoke->NbrEntries--;
    }
}

/*$PAGE*/
/*
************************************************************************************************************************
*                                              RESET TICK LIST PEAK DETECTOR
*
* Description: This function is used to reset the peak detector for the number of entries in each spoke.
*
* Arguments  : void
*
* Returns    : none
*
* Note(s)    : This function is INTERNAL to uC/OS-III and your application should not call it.
************************************************************************************************************************
*/


void  OS_TickListResetPeak (void)
{
    OS_TICK_SPOKE_IX   i;
    OS_TICK_SPOKE     *p_spoke;



    for (i = 0u; i < OSCfg_TickWheelSize; i++) {
        p_spoke                = (OS_TICK_SPOKE *)&OSCfg_TickWheel[i];
        p_spoke->NbrEntriesMax = (OS_OBJ_QTY     )0u;
    }
}

/*$PAGE*/
/*
************************************************************************************************************************
*                                                UPDATE THE TICK LIST
*
* Description: This function is called when a tick occurs and determines if the timeout waiting for a kernel object has
*              expired or a delay has expired.
*
* Arguments  : non
*
* Returns    : none
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

void  OS_TickListUpdate (void)
{

  
    CPU_BOOLEAN        done;
    OS_TICK_SPOKE     *p_spoke;
    OS_TCB            *p_tcb;
    OS_TCB            *p_tcb_next;
    OS_TICK_SPOKE_IX   spoke;
    CPU_TS             ts_start;
    CPU_TS             ts_end;
    CPU_SR_ALLOC();


    OS_CRITICAL_ENTER();
    ts_start = OS_TS_GET();
    OSTickCtr++;                                                       /* Keep track of the number of ticks           */
    spoke    = (OS_TICK_SPOKE_IX)(OSTickCtr % OSCfg_TickWheelSize);
    p_spoke  = &OSCfg_TickWheel[spoke];
    p_tcb    = p_spoke->FirstPtr;
    done     = DEF_FALSE;
    while (done == DEF_FALSE) {
        if (p_tcb != (OS_TCB *)0) {
            p_tcb_next = p_tcb->TickNextPtr;                           /* Point to next TCB to update                 */
            switch (p_tcb->TaskState) {
                case OS_TASK_STATE_RDY:
                case OS_TASK_STATE_PEND:
                case OS_TASK_STATE_SUSPENDED:
                case OS_TASK_STATE_PEND_SUSPENDED:
                     break;

                case OS_TASK_STATE_DLY:
                     p_tcb->TickRemain = p_tcb->TickCtrMatch           /* Compute time remaining of current TCB       */
                                       - OSTickCtr;
                     if (OSTickCtr == p_tcb->TickCtrMatch) {           /* Process each TCB that expires               */
                         p_tcb->TaskState = OS_TASK_STATE_RDY;
                         OS_TaskRdy(p_tcb);                            /* Make task ready to run                      */
                     } else {
                         done             = DEF_TRUE;                  /* Don't find a match, we're done!             */
                     }
                     break;

                case OS_TASK_STATE_PEND_TIMEOUT:
                     p_tcb->TickRemain = p_tcb->TickCtrMatch           /* Compute time remaining of current TCB       */
                                       - OSTickCtr;
                     if (OSTickCtr == p_tcb->TickCtrMatch) {           /* Process each TCB that expires               */
#if (OS_MSG_EN > 0u)
                         p_tcb->MsgPtr     = (void      *)0;
                         p_tcb->MsgSize    = (OS_MSG_SIZE)0u;
#endif
                         p_tcb->TS         = OS_TS_GET();
                         OS_PendListRemove(p_tcb);                     /* Remove from wait list                       */
                         OS_TaskRdy(p_tcb);
                         p_tcb->TaskState  = OS_TASK_STATE_RDY;
                         p_tcb->PendStatus = OS_STATUS_PEND_TIMEOUT;   /* Indicate pend timed out                     */
                         p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;  /* Indicate no longer pending                  */
                     } else {
                         done              = DEF_TRUE;                 /* Don't find a match, we're done!             */
                     }
                     break;

                case OS_TASK_STATE_DLY_SUSPENDED:
                     p_tcb->TickRemain = p_tcb->TickCtrMatch           /* Compute time remaining of current TCB       */
                                       - OSTickCtr;
                     if (OSTickCtr == p_tcb->TickCtrMatch) {           /* Process each TCB that expires               */
                         p_tcb->TaskState  = OS_TASK_STATE_SUSPENDED;
                         OS_TickListRemove(p_tcb);                     /* Remove from current wheel spoke             */
                     } else {
                         done              = DEF_TRUE;                 /* Don't find a match, we're done!             */
                     }
                     break;

                case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:
                     p_tcb->TickRemain = p_tcb->TickCtrMatch           /* Compute time remaining of current TCB       */
                                       - OSTickCtr;
                     if (OSTickCtr == p_tcb->TickCtrMatch) {           /* Process each TCB that expires               */
#if (OS_MSG_EN > 0u)
                         p_tcb->MsgPtr     = (void      *)0;
                         p_tcb->MsgSize    = (OS_MSG_SIZE)0u;
#endif
                         p_tcb->TS         = OS_TS_GET();
                         OS_PendListRemove(p_tcb);                     /* Remove from wait list                       */
                         OS_TickListRemove(p_tcb);                     /* Remove from current wheel spoke             */
                         p_tcb->TaskState  = OS_TASK_STATE_SUSPENDED;
                         p_tcb->PendStatus = OS_STATUS_PEND_TIMEOUT;   /* Indicate pend timed out                     */
                         p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;  /* Indicate no longer pending                  */
                     } else {
                         done              = DEF_TRUE;                 /* Don't find a match, we're done!             */
                     }
                     break;

                default:
                     break;
            }
            p_tcb = p_tcb_next;
        } else {
            done  = DEF_TRUE;
        }
    }
    ts_end = OS_TS_GET() - ts_start;                                   /* Measure execution time of tick task         */
    if (ts_end > OSTickTaskTimeMax) {
        OSTickTaskTimeMax = ts_end;
    }

    OS_CRITICAL_EXIT();
}
