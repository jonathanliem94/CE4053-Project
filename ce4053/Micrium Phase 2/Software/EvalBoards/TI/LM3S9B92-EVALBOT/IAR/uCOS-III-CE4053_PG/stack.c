#include <stack.h>
 

struct node
{
    int data;
    struct node* next;
};
 
/*
    init the stack
*/
void init(struct node* head)
{
    head = 0;
}
 
/*
    push an element into stack
*/
struct node* push(struct node* head,int data)
{
    struct node* tmp = (struct node*)memget(sizeof(struct node));
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
struct node* pop(struct node *head,int *element)
{
    struct node* tmp = head;
    *element = head->data;
    head = head->next;
    free(tmp);
    return head;
}
/*
    returns 1 if the stack is empty, otherwise returns 0
*/
int empty(struct node* head)
{
    return head == 0 ? 1 : 0;
}
 