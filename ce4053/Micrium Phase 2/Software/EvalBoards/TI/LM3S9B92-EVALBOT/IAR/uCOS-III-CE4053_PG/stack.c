#include <stack.h>

/*
    init the stack
*/
void stack_init(struct stack_node* head)
{
    head = 0;
}
 
/*
    push an element into stack
*/
struct stack_node* stack_push(struct stack_node* head,OS_MUTEX* data)
{
    struct stack_node* tmp = (struct stack_node*)memget(sizeof(struct stack_node));
    if(tmp == 0)
    {
        exit(0);
    }
    tmp->data = data;
    tmp->next = head;
    head = tmp;
    return head;
}
/*
    pop an element from the stack
*/
struct stack_node* stack_pop(struct stack_node *head,OS_MUTEX *element)
{
    struct stack_node* tmp = head;
    element = head->data;
    head = head->next;
    free(tmp);
    return head;
}

/*   find min deadlines of all mutexes stored inside stack */
OS_DEADLINE stack_find_min_deadline(struct stack_node *head)
{
  OS_DEADLINE deadline = 0;
  struct stack_node* cur = head;
  deadline = cur->data->Resource_Ceiling->Deadline;
  /*    node is given
    we iterate through each by having current pointer = head
    while head is not null we check its deadline value (since each node has Mutex data to check TCB deadline value)
    
  */
  while (cur->next != 0){
    cur = cur->next;
    if (deadline > cur->data->Resource_Ceiling->Deadline) {
      deadline = cur->data->Resource_Ceiling->Deadline;
    }
  }
  return deadline;
}