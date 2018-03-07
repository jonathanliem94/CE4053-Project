/*
************************************************************************************************************************
*                                                      uC/OS-III
*                                                 The Real-Time Kernel
*
*                                  (c) Copyright 2009-2011; Micrium, Inc.; Weston, FL
*                           All rights reserved.  Protected by international copyright laws.
*
*                                                 PRIORITY MANAGEMENT
*
* File    : OS_PRIO.C
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

#include  <os.h>

#ifdef VSC_INCLUDE_SOURCE_FILE_NAMES
const  CPU_CHAR  *os_prio__c = "$Id: $";
#endif


CPU_DATA   OSPrioTbl[OS_PRIO_TBL_SIZE];                     /* Declare the array local to this file to allow for  ... */
OS_AVL_Node                *OS_PrioAVL_root;                                                      /* ... optimization.  In other words, this allows the ... */
                                                            /* ... table to be located in fast memory                 */

/*
************************************************************************************************************************
*                                               INITIALIZE THE PRIORITY LIST
*
* Description: This function is called by uC/OS-III to initialize the list of ready priorities.
*
* Arguments  : none
*
* Returns    : none
*
* Note       : This function is INTERNAL to uC/OS-III and your application should not call it.
************************************************************************************************************************
*/

void  OS_PrioInit (void)
{
    OS_PrioAVL_root=0;
}

/*
************************************************************************************************************************
*                                           GET HIGHEST PRIORITY TASK WAITING
*
* Description: This function is called by other uC/OS-III services to determine the highest priority task
*              waiting on the event.
*
* Arguments  : none
*
* Returns    : The priority of the Highest Priority Task (HPT) waiting for the event
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

OS_PRIO  OS_PrioGetHighest (OS_AVL_Node* t)
{
    if( t == 0 )
        return 0;
    else if( t->AVL_node_left == 0 )
        return t->priority;
    else
        return OS_PrioGetHighest( t->AVL_node_left );
}

/*
************************************************************************************************************************
*                                                  INSERT PRIORITY
*
* Description: This function is called to insert a priority in the priority table.
*
* Arguments  : prio     is the priority to insert
*
* Returns    : none
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

OS_AVL_Node*  OS_PrioInsert (OS_AVL_Node *T, OS_PRIO  prio)
{
    if(T==0)
    {
        T=(OS_AVL_Node*)malloc(sizeof(OS_AVL_Node));
        T->priority=prio;
        T->AVL_node_left=0;
        T->AVL_node_right=0;
    }
    else
        if(prio > T->priority)        // insert in right subtree
        {
            T->AVL_node_right=OS_PrioInsert(T->AVL_node_right,prio);
            if(OS_AVL_BF(T)==-2)
                if(prio>T->AVL_node_right->priority)
                    T=OS_AVL_RR(T);
                else
                    T=OS_AVL_RL(T);
        }
        else
            if(prio<T->priority)
            {
                T->AVL_node_left=OS_PrioInsert(T->AVL_node_left,prio);
                if(OS_AVL_BF(T)==2)
                    if(prio < T->AVL_node_left->priority)
                        T=OS_AVL_LL(T);
                    else
                        T=OS_AVL_LR(T);
            }
        
        T->height=OS_AVL_height(T);
        
        return(T);
}

/*
************************************************************************************************************************
*                                                   REMOVE PRIORITY
*
* Description: This function is called to remove a priority in the priority table.
*
* Arguments  : prio     is the priority to remove
*
* Returns    : none
*
* Note(s)    : 1) This function is INTERNAL to uC/OS-III and your application MUST NOT call it.
************************************************************************************************************************
*/

OS_AVL_Node*  OS_PrioRemove (OS_AVL_Node *T, OS_PRIO  prio)
{
    OS_AVL_Node *p;
    if(T==0)
    {
        return 0;
    }
    else
        if(prio > T->priority)        // insert in right subtree
        {
            T->AVL_node_right=OS_PrioRemove(T->AVL_node_right,prio);
            if(OS_AVL_BF(T)==2)
                if(OS_AVL_BF(T->AVL_node_left)>=0)
                    T=OS_AVL_LL(T);
                else
                    T=OS_AVL_LR(T);
        }
        else
            if(prio<T->priority)
            {
                T->AVL_node_left=OS_PrioRemove(T->AVL_node_left,prio);
                if(OS_AVL_BF(T)==-2)    //Rebalance during windup
                    if(OS_AVL_BF(T->AVL_node_right)<=0)
                        T=OS_AVL_RR(T);
                    else
                        T=OS_AVL_RL(T);
            }
            else
            {
                //prio to be deleted is found
                if(T->AVL_node_right!=0)
                {    //delete its inorder succesor
                    p=T->AVL_node_right;
                    
                    while(p->AVL_node_left!= 0)
                        p=p->AVL_node_left;
                    
                    T->priority=p->priority;
                    T->AVL_node_right=OS_PrioRemove(T->AVL_node_right,p->priority);
                    
                    if(OS_AVL_BF(T)==2)//Rebalance during windup
                        if(OS_AVL_BF(T->AVL_node_left)>=0)
                            T=OS_AVL_LL(T);
                        else
                            T=OS_AVL_LR(T);\
                }
                else
                    return(T->AVL_node_left);
            }
    T->height=OS_AVL_height(T);
    return(T);
}
/*
************************************************************************************************************************
*                                                   NEW FUNCTIONS
************************************************************************************************************************
*/

CPU_INT08U OS_AVL_height(OS_AVL_Node *T)
{
    CPU_INT08U lh,rh;
    if(T==0)
        return(0);
    
    if(T->AVL_node_left==0)
        lh=0;
    else
        lh=1+T->AVL_node_left->height;
        
    if(T->AVL_node_right==0)
        rh=0;
    else
        rh=1+T->AVL_node_right->height;
    
    if(lh>rh)
        return(lh);
    
    return(rh);
}
 
OS_AVL_Node *OS_AVL_rotateright(OS_AVL_Node *x)
{
    OS_AVL_Node *y;
    y=x->AVL_node_left;
    x->AVL_node_left=y->AVL_node_right;
    y->AVL_node_right=x;
    x->height=OS_AVL_height(x);
    y->height=OS_AVL_height(y);
    return(y);
}

OS_AVL_Node *OS_AVL_rotateleft(OS_AVL_Node *x)
{
    OS_AVL_Node *y;
    y=x->AVL_node_right;
    x->AVL_node_right=y->AVL_node_left;
    y->AVL_node_left=x;
    x->height=OS_AVL_height(x);
    y->height=OS_AVL_height(y);
    
    return(y);
}

OS_AVL_Node *OS_AVL_RR(OS_AVL_Node *T)
{
    T=OS_AVL_rotateleft(T);
    return(T);
}

OS_AVL_Node *OS_AVL_LL(OS_AVL_Node *T)
{
    T=OS_AVL_rotateright(T);
    return(T);
}

OS_AVL_Node *OS_AVL_LR(OS_AVL_Node *T)
{
    T->AVL_node_left=OS_AVL_rotateleft(T->AVL_node_left);
    T=OS_AVL_rotateright(T);
    
    return(T);
}

OS_AVL_Node *OS_AVL_RL(OS_AVL_Node *T)
{
    T->AVL_node_right=OS_AVL_rotateright(T->AVL_node_right);
    T=OS_AVL_rotateleft(T);
    return(T);
}

/* Balance Factor */
CPU_INT08U OS_AVL_BF(OS_AVL_Node *T)
{
    int lh,rh;
    if(T==0)
        return(0);
 
    if(T->AVL_node_left==0)
        lh=0;
    else
        lh=1+T->AVL_node_left->height;
 
    if(T->AVL_node_right==0)
        rh=0;
    else
        rh=1+T->AVL_node_right->height;
 
    return(lh-rh);
}