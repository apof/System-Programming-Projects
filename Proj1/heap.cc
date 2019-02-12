#include "heap.h"

/*SOURCE:http://stackoverflow.com/questions/19720438/pointer-based-binary-heap-implementation*/

void Heap::swap (HeapNode *n)
{
 	HeapNode *p = n->parent;

	if (p->parent)	p->parent->link[p == p->parent->link[1]] = n;
  	else		root = n;

	n->parent = p->parent;

    	int nside = (n == p->link[1]);

    	HeapNode *c = p->link[!nside];

	p->link[0] = n->link[0];
	if (p->link[0])
        p->link[0]->parent = p;


	p->link[1] = n->link[1];
	if (p->link[1])	p->link[1]->parent = p;

	n->link[nside] = p;
	p->parent = n;

	n->link[!nside] = c;
	if (c)	c->parent = n;

	if (n == last)	last = p;
}


void Heap::Insert (char* c, float mon)
        {

		total_cost+=mon;

		HeapNode* node;

		HeapNode* found = find(c,mon);

		if (found==NULL)
		{

		node = new HeapNode(c,mon);

                if (root == NULL)
                {
                        root = node;
                        last = node;
                        node->parent = NULL;
                        node->link[0] = NULL;
                        node->link[1] = NULL;
                        return;
                }

                HeapNode *cur = last;
                while (cur->parent != NULL && cur == cur->parent->link[1]) {    cur = cur->parent;      }


		if (cur->parent != NULL) {
                        if (cur->parent->link[1] != NULL) {
                        cur = cur->parent->link[1];
                        while (cur->link[0] != NULL) {
                                cur = cur->link[0];
                        }
                        } else {
                        cur = cur->parent;
                }
                } else {
                while (cur->link[0] != NULL) {
                cur = cur->link[0];
                }

		}

                last = node;
                cur->link[cur->link[0] != NULL] = node;
                node->parent = cur;
                node->link[0] = NULL;
                node->link[1] = NULL;

		}

		if (found!=NULL) node = found;
                while (node->parent != NULL && node->parent->money < node->money )	swap(node);
        }



