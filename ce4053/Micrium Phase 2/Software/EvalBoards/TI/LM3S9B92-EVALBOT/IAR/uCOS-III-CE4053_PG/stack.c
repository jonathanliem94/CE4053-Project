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
struct stack_node* stack_push(struct stack_node* head,struct stack_node* node_to_be_inserted)
{
//    struct stack_node* tmp = (struct stack_node*)memget(sizeof(struct stack_node));
//    if(node_to_be_inserted == 0)
//    {
//        exit(0);
////    }
//    tmp->data = data;
  if (head==0)
  {
    head = node_to_be_inserted;
  }
  else
  {
    node_to_be_inserted->next = head;
    head = node_to_be_inserted;
  }
   return head;
}
/*
    pop an element from the stack
*/
struct stack_node* stack_pop(struct stack_node *head)
{

    head = head->next;
//    free(tmp);
    return head;
}

/*   find min deadlines of all mutexes stored inside stack */
OS_DEADLINE stack_find_min_deadline(struct stack_node *head)
{
  OS_DEADLINE deadline = 99999999;
  struct stack_node* cur = head;
  
  /*    node is given
    we iterate through each by having current pointer = head
    while head is not null we check its deadline value (since each node has Mutex data to check TCB deadline value)
    
  */
  if(cur==0)    //      nothing in mutex
  {
    return 99999999;
  }
  else {        //      something in mutex
    deadline = cur->data->Resource_Ceiling->Deadline;
    while (cur != 0){
      if (cur->data->Resource_Ceiling->Deadline < deadline) {
        deadline = cur->data->Resource_Ceiling->Deadline;
      }
      cur = cur->next;
    }
  }
  return deadline;
}