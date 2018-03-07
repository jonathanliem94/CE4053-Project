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
AVL_Node                *AVL_root;                                                      /* ... optimization.  In other words, this allows the ... */
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
    AVL_root=0;
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

OS_PRIO  OS_PrioGetHighest (AVL_Node* t)
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

AVL_Node*  OS_PrioInsert (AVL_Node *T, OS_PRIO  prio)
{
    if(T==0)
    {
        T=(AVL_Node*)malloc(sizeof(AVL_Node));
        T->priority=prio;
        T->AVL_node_left=0;
        T->AVL_node_right=0;
    }
    else
        if(prio > T->priority)        // insert in right subtree
        {
            T->AVL_node_right=OS_PrioInsert(T->AVL_node_right,prio);
            if(BF(T)==-2)
                if(prio>T->AVL_node_right->priority)
                    T=AVL_RR(T);
                else
                    T=AVL_RL(T);
        }
        else
            if(prio<T->priority)
            {
                T->AVL_node_left=OS_PrioInsert(T->AVL_node_left,prio);
                if(BF(T)==2)
                    if(prio < T->AVL_node_left->priority)
                        T=AVL_LL(T);
                    else
                        T=AVL_LR(T);
            }
        
        T->height=AVL_height(T);
        
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

AVL_Node*  OS_PrioRemove (AVL_Node *T, OS_PRIO  prio)
{
    AVL_Node *p;
    if(T==0)
    {
        return 0;
    }
    else
        if(prio > T->priority)        // insert in right subtree
        {
            T->AVL_node_right=OS_PrioRemove(T->AVL_node_right,prio);
            if(BF(T)==2)
                if(BF(T->AVL_node_left)>=0)
                    T=AVL_LL(T);
                else
                    T=AVL_LR(T);
        }
        else
            if(prio<T->priority)
            {
                T->AVL_node_left=OS_PrioRemove(T->AVL_node_left,prio);
                if(BF(T)==-2)    //Rebalance during windup
                    if(BF(T->AVL_node_right)<=0)
                        T=AVL_RR(T);
                    else
                        T=AVL_RL(T);
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
                    
                    if(BF(T)==2)//Rebalance during windup
                        if(BF(T->AVL_node_left)>=0)
                            T=AVL_LL(T);
                        else
                            T=AVL_LR(T);\
                }
                else
                    return(T->AVL_node_left);
            }
    T->height=AVL_height(T);
    return(T);
}
/*
************************************************************************************************************************
*                                                   NEW FUNCTIONS
************************************************************************************************************************
*/

CPU_INT08U AVL_height(AVL_Node *T)
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
 
AVL_Node *AVL_rotateright(AVL_Node *x)
{
    AVL_Node *y;
    y=x->AVL_node_left;
    x->AVL_node_left=y->AVL_node_right;
    y->AVL_node_right=x;
    x->height=AVL_height(x);
    y->height=AVL_height(y);
    return(y);
}

AVL_Node *AVL_rotateleft(AVL_Node *x)
{
    AVL_Node *y;
    y=x->AVL_node_right;
    x->AVL_node_right=y->AVL_node_left;
    y->AVL_node_left=x;
    x->height=AVL_height(x);
    y->height=AVL_height(y);
    
    return(y);
}

AVL_Node *AVL_RR(AVL_Node *T)
{
    T=AVL_rotateleft(T);
    return(T);
}

AVL_Node *AVL_LL(AVL_Node *T)
{
    T=AVL_rotateright(T);
    return(T);
}

AVL_Node *AVL_LR(AVL_Node *T)
{
    T->AVL_node_left=AVL_rotateleft(T->AVL_node_left);
    T=AVL_rotateright(T);
    
    return(T);
}

AVL_Node *AVL_RL(AVL_Node *T)
{
    T->AVL_node_right=AVL_rotateright(T->AVL_node_right);
    T=AVL_rotateleft(T);
    return(T);
}

/* Balance Factor */
CPU_INT08U BF(AVL_Node *T)
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