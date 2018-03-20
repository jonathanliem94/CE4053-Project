#include <os.h>
struct node{
	int period;
        int deadline;
	OS_TCB *p_tcb;
};

struct heap
{
	unsigned int size; // Size of the allocated memory (in number of items)
	unsigned int count; // Count of the elements in the heap
	struct node *node_arr[5]; // Array with the elements
};
void heap_init(struct heap *h);
void heap_push(struct heap *h, struct node *node_arr);
void heap_pop(struct heap *h);

// Returns the biggest element in the heap
#define heap_front(h) (*(h)->node_arr)

// Frees the allocated memory
#define heap_term(h) (free((h)->node_arr))

void heapify(int period[], unsigned int count);