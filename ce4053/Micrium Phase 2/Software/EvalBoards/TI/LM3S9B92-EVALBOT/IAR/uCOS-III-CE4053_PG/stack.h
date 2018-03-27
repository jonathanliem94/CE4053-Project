#include <os.h>
struct stack_node
{
    OS_MUTEX* data;
    struct stack_node* next;
};
OS_DEADLINE stack_find_min_deadline(struct stack_node *head);
struct stack_node* stack_push(struct stack_node *s,OS_MUTEX *element);
struct stack_node* stack_pop(struct stack_node *s,OS_MUTEX *element);
void stack_init(struct stack_node* s);