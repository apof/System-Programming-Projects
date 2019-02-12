#include <iostream>
#include <assert.h>
#include <cstring>

using namespace std;

struct HeapNode {

	HeapNode *parent;
    	HeapNode *link[2];
	char* caller;
	float money;
	int flag;

	HeapNode(char* c,float mon)
		{

		caller = new char[strlen(c) + 1];
		strcpy(caller,c);

		money = mon;

		flag=0;
		}

	~HeapNode() { delete caller; }

};


class Heap {

	private:

	HeapNode *root;
    	HeapNode *last;

	float total_cost;

	void swap(HeapNode*);

	void destroy_tree(HeapNode* leaf)
	{
		if(leaf!=NULL)
		{
			destroy_tree(leaf->link[0]);
			destroy_tree(leaf->link[1]);
			delete leaf;
		}
	}

	HeapNode* find(HeapNode* leaf,char* c,float mon)
		{
			HeapNode* found;

			if(leaf!=NULL)
			{
			if (strcmp(leaf->caller,c)==0) { leaf->money+=mon; found = leaf; return found; }
			found = find(leaf->link[0],c,mon); { if (found!=NULL) return found; }
			found = find(leaf->link[1],c,mon); { return found; }
			}
			return NULL;
		}

	void heap_inorder_print(HeapNode* leaf)
        {
                if(leaf)
                        {
                        heap_inorder_print(leaf->link[0]);
                        heap_inorder_print(leaf->link[1]);
                        cout<< leaf->caller <<" "<< leaf->money<<" "<< leaf->flag<<endl;
                        }

        }

	void heap_initialize(HeapNode* leaf)
        {
                if(leaf)
                        {
                        heap_initialize(leaf->link[0]);
                        heap_initialize(leaf->link[1]);
                        leaf->flag=0;
                        }

        }

	void top(HeapNode* leaf, HeapNode*& temp)
	{
		if(leaf)
		{
		if (leaf->money >= temp->money && leaf->flag!=1 ) { temp = leaf; }
		top(leaf->link[0],temp);
		top(leaf->link[1],temp);
		}
	}

	public:

	Heap() { root = NULL; last = NULL; total_cost = 0; }

	~Heap()
        {
                destroy_tree(root);
        }


	void Insert (char*,float);

	HeapNode* find(char* caller, float money) { return find(root,caller,money); }

	void heap_inorder_print() { heap_inorder_print(root); }

	void print_top() { cout<< "---->" << root->caller << "--" << root->money <<" "<<root->flag <<endl; }

	void clear() { if (root!=NULL) destroy_tree(root); root = NULL; total_cost = 0.0; }

	void heap_initialize() { heap_initialize(root); }

	void top(int percent) 
	{ 
		float number = (percent*total_cost)/100;
		cout<<number<<" "<<percent<<" "<<total_cost<<endl; 
		char buff[5];
		strcpy(buff,"a");
		float cost=0.0; 

		HeapNode* node = new HeapNode(buff,0);
		HeapNode* node2;

		int first = 1;

		while(cost<number) {
		
		node2 = node;

		top(root,node2);
		node2->flag = 1;
		if (strcmp("a",node2->caller)==0) return;
		cout<<"-->"<<node2->caller<<" "<<node2->money<<endl;
		cost+=node2->money;

		}

		delete node;

	}		 
};
