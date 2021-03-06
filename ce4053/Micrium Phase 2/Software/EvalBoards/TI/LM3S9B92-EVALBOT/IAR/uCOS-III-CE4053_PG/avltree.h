#include "stddef.h"
#include "stdint.h"
#include <os.h>
#ifdef __cplusplus
extern "C" {
#endif

struct avl_node {
    struct avl_node *parent, *left, *right;
    int tcb_count;
#ifdef _AVL_SEPARATE_PARENT_BF
    int bf;
#endif
#ifdef _AVL_NEXT_POINTER
    struct avl_node *prev, *next;
#endif
};

struct os_avl_node{
    struct avl_node avl;
    OS_DEADLINE deadline;
    OS_TCB* p_tcb1;
    OS_TCB* p_tcb2;
    OS_TCB* p_tcb3;
    //maybe use array to store (but need change alot of other things
     // p_tcb[5] means each avl node can contain 5 slots, then if they have same deadline
    //  can put in same array set
    //  so if 2 out of 3 same deadline, it will be: 1 node with array (inside just 1 tcb), 1 node with array (inside 2 tcb with same array)
    //  if all diff deadline, 3 nodes with array(each 1 tcb)
};

struct avl_tree{
    struct avl_node *root;
    void *aux;
};

#ifndef _get_entry
#define _get_entry(ELEM, STRUCT, MEMBER)                              \
        ((STRUCT *) ((uint8_t *) (ELEM) - offsetof (STRUCT, MEMBER)))
#endif

#define avl_parent(node) \
        ((struct avl_node *)((uint64_t)(node)->parent & ~0x3))

#ifdef _AVL_SEPARATE_PARENT_BF
    #define avl_bf(node) ((node)->bf)
#else
    #define avl_bf(node) (((int)((uint64_t)(node)->parent & 0x3)) - 1)
#endif

// *a < *b : return neg
// *a == *b : return 0
// *a > *b : return pos
typedef int avl_cmp_func (struct avl_node *a, struct avl_node *b, void *aux);

int cmp_func(struct avl_node *a, struct avl_node *b, void *aux);

void avl_init(struct avl_tree *tree, void *aux);
void avl_set_aux(struct avl_tree *tree, void *aux);
struct avl_node* avl_insert(struct avl_tree *tree,
                            struct avl_node *node,
                            avl_cmp_func *func);
struct avl_node* avl_search(struct avl_tree *tree,
                            struct avl_node *node,
                            avl_cmp_func *func);
struct avl_node* avl_search_greater(struct avl_tree *tree,
                                    struct avl_node *node,
                                    avl_cmp_func *func);
struct avl_node* avl_search_smaller(struct avl_tree *tree,
                                    struct avl_node *node,
                                    avl_cmp_func *func);
void avl_remove(struct avl_tree *tree,
                struct avl_node *node);
struct avl_node* avl_first(struct avl_tree *tree);
struct avl_node* avl_last(struct avl_tree *tree);
struct avl_node* avl_next(struct avl_node *node);
struct avl_node* avl_prev(struct avl_node *node);
#ifdef __cplusplus
}
#endif

