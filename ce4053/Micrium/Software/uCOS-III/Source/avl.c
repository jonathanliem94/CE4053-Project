/*
************************************************************************************************************************
*                                                   NEW FUNCTIONS
************************************************************************************************************************
*/
#include  <os.h>
//
//OS_AVL_Node                *OS_PrioAVL_root;
//
//OS_AVL_Node*  OS_AVLRemove (OS_AVL_Node *T, OS_PRIO  prio)
//{
//    OS_AVL_Node *p;
//    if(T==0)
//    {
//        return 0;
//    }
//    else
//        if(prio > T->priority)        // insert in right subtree
//        {
//            T->AVL_node_right=OS_AVLRemove(T->AVL_node_right,prio);
//            if(OS_AVL_BF(T)==2)
//                if(OS_AVL_BF(T->AVL_node_left)>=0)
//                    T=OS_AVL_LL(T);
//                else
//                    T=OS_AVL_LR(T);
//        }
//        else
//            if(prio<T->priority)
//            {
//                T->AVL_node_left=OS_AVLRemove(T->AVL_node_left,prio);
//                if(OS_AVL_BF(T)==-2)    //Rebalance during windup
//                    if(OS_AVL_BF(T->AVL_node_right)<=0)
//                        T=OS_AVL_RR(T);
//                    else
//                        T=OS_AVL_RL(T);
//            }
//            else
//            {
//                //prio to be deleted is found
//                if(T->AVL_node_right!=0)
//                {    //delete its inorder succesor
//                    p=T->AVL_node_right;
//                    
//                    while(p->AVL_node_left!= 0)
//                        p=p->AVL_node_left;
//                    
//                    T->priority=p->priority;
//                    T->AVL_node_right=OS_PrioRemove(T->AVL_node_right,p->priority);
//                    
//                    if(OS_AVL_BF(T)==2)//Rebalance during windup
//                        if(OS_AVL_BF(T->AVL_node_left)>=0)
//                            T=OS_AVL_LL(T);
//                        else
//                            T=OS_AVL_LR(T);\
//                }
//                else
//                    return(T->AVL_node_left);
//            }
//    T->height=OS_AVL_height(T);
//    return(T);
//}
//
//OS_AVL_Node*  OS_PrioRemove (OS_AVL_Node *T, OS_PRIO  prio)
//{
//    OS_AVL_Node *p;
//    if(T==0)
//    {
//        return 0;
//    }
//    else
//        if(prio > T->priority)        // insert in right subtree
//        {
//            T->AVL_node_right=OS_PrioRemove(T->AVL_node_right,prio);
//            if(OS_AVL_BF(T)==2)
//                if(OS_AVL_BF(T->AVL_node_left)>=0)
//                    T=OS_AVL_LL(T);
//                else
//                    T=OS_AVL_LR(T);
//        }
//        else
//            if(prio<T->priority)
//            {
//                T->AVL_node_left=OS_PrioRemove(T->AVL_node_left,prio);
//                if(OS_AVL_BF(T)==-2)    //Rebalance during windup
//                    if(OS_AVL_BF(T->AVL_node_right)<=0)
//                        T=OS_AVL_RR(T);
//                    else
//                        T=OS_AVL_RL(T);
//            }
//            else
//            {
//                //prio to be deleted is found
//                if(T->AVL_node_right!=0)
//                {    //delete its inorder succesor
//                    p=T->AVL_node_right;
//                    
//                    while(p->AVL_node_left!= 0)
//                        p=p->AVL_node_left;
//                    
//                    T->priority=p->priority;
//                    T->AVL_node_right=OS_PrioRemove(T->AVL_node_right,p->priority);
//                    
//                    if(OS_AVL_BF(T)==2)//Rebalance during windup
//                        if(OS_AVL_BF(T->AVL_node_left)>=0)
//                            T=OS_AVL_LL(T);
//                        else
//                            T=OS_AVL_LR(T);\
//                }
//                else
//                    return(T->AVL_node_left);
//            }
//    T->height=OS_AVL_height(T);
//    return(T);
//}
//
//CPU_INT08U OS_AVL_height(OS_AVL_Node *T)
//{
//    CPU_INT08U lh,rh;
//    if(T==0)
//        return(0);
//    
//    if(T->AVL_node_left==0)
//        lh=0;
//    else
//        lh=1+T->AVL_node_left->height;
//        
//    if(T->AVL_node_right==0)
//        rh=0;
//    else
//        rh=1+T->AVL_node_right->height;
//    
//    if(lh>rh)
//        return(lh);
//    
//    return(rh);
//}
// 
//OS_AVL_Node *OS_AVL_rotateright(OS_AVL_Node *x)
//{
//    OS_AVL_Node *y;
//    y=x->AVL_node_left;
//    x->AVL_node_left=y->AVL_node_right;
//    y->AVL_node_right=x;
//    x->height=OS_AVL_height(x);
//    y->height=OS_AVL_height(y);
//    return(y);
//}
//
//OS_AVL_Node *OS_AVL_rotateleft(OS_AVL_Node *x)
//{
//    OS_AVL_Node *y;
//    y=x->AVL_node_right;
//    x->AVL_node_right=y->AVL_node_left;
//    y->AVL_node_left=x;
//    x->height=OS_AVL_height(x);
//    y->height=OS_AVL_height(y);
//    
//    return(y);
//}
//
//OS_AVL_Node *OS_AVL_RR(OS_AVL_Node *T)
//{
//    T=OS_AVL_rotateleft(T);
//    return(T);
//}
//
//OS_AVL_Node *OS_AVL_LL(OS_AVL_Node *T)
//{
//    T=OS_AVL_rotateright(T);
//    return(T);
//}
//
//OS_AVL_Node *OS_AVL_LR(OS_AVL_Node *T)
//{
//    T->AVL_node_left=OS_AVL_rotateleft(T->AVL_node_left);
//    T=OS_AVL_rotateright(T);
//    
//    return(T);
//}
//
//OS_AVL_Node *OS_AVL_RL(OS_AVL_Node *T)
//{
//    T->AVL_node_right=OS_AVL_rotateright(T->AVL_node_right);
//    T=OS_AVL_rotateleft(T);
//    return(T);
//}
//
///* Balance Factor */
//CPU_INT08U OS_AVL_BF(OS_AVL_Node *T)
//{
//    int lh,rh;
//    if(T==0)
//        return(0);
// 
//    if(T->AVL_node_left==0)
//        lh=0;
//    else
//        lh=1+T->AVL_node_left->height;
// 
//    if(T->AVL_node_right==0)
//        rh=0;
//    else
//        rh=1+T->AVL_node_right->height;
// 
//    return(lh-rh);
//}