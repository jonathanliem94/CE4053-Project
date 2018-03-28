#include <os.h>

#define RED 1
#define BLACK 0

#define LEFT 1
#define RIGHT 2

#define IS_NULL(node) ((node) == 0)
#define IS_RED(node) ((node) != 0 && (node)->color == RED)

typedef struct RBNode {
	struct RBNode *parent;
	void *key;
	OS_TCB *value;
	struct RBNode *left;
	struct RBNode *right;
	int color;
} RBNode;

typedef struct RBTree {
	struct RBNode *root;
	int (*rbt_keycmp)(void *, void *);
} RBTree;

void rbtree_init(RBTree* tree, int (*rbt_keycmp)(void *, void *));
void rbtree_insert(RBTree *tree, RBNode *new_node);
void *rbtree_del(RBTree *tree, void *key);
struct RBNode *_rbtree_minimum(struct RBNode *node);
