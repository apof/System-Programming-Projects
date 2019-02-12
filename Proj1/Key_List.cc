#include "header.h"

void Key_List::Insert_List(char* k, CDR_Record* rec,int rec_num) {

	Key_Node* tmp = header;
        int pos;

	if (header == NULL) { header = new Key_Node(key_number,rec_num); header->Insert_key(k,rec,0); }
	else
	{

	while(tmp->next!=NULL) {

	pos=tmp->key_already_exists(k);
        if (pos!=-1) { tmp->Insert_key(k,rec,pos); return; }

	tmp = tmp->next;

	}


	pos=tmp->key_already_exists(k);
        if (pos!=-1) { tmp->Insert_key(k,rec,pos); return; }

	pos=tmp->Free_Space();
	if (pos!=-1) { tmp->Insert_key(k,rec,pos); return; }

	tmp->next = new Key_Node(key_number,rec_num); tmp->next->Insert_key(k,rec,0);

	}

	return;

}


Key_List::~Key_List() {

	Key_Node* tmp = header;
	Key_Node* tmp1;
	while(tmp!=NULL)
	{
	tmp1=tmp->next;
	delete tmp;
	tmp = tmp1;
	}
}

void Key_List::Print_List() {

	Key_Node* tmp = header;
	while(tmp!=NULL) { tmp->Print_key_table(); tmp=tmp->next; }
}


void Key_List::delet(char* id, char* caller) {

	Key_Node* tmp = header;
	Key_Node* tmp1 = header;
        int pos;

	if (tmp!=NULL)
	{
	while(tmp->key_already_exists(caller)==-1) {

	tmp1=tmp;
	tmp=tmp->next;
	if(tmp == NULL)	break;
	}

	if (tmp!=NULL)
	{

        pos=tmp->key_already_exists(caller);
        if(tmp->key_table[pos]->list->delete_record(id)) { delete tmp->key_table[pos]->key; tmp->key_table[pos]->key = NULL; }
	

	if (tmp->is_empty())
	{
		if(tmp1=tmp)
		header = tmp1->next;
		else
		tmp1->next = tmp->next;
		delete tmp;
	}
	}
	}
	else cout<<"not found"<<endl;
}

void Key_List::find(char* number, char* time1, char* year1, char* time2 , char* year2) {

	Key_Node* tmp = header;
        int pos;

        while(tmp->next!=NULL) {

        pos=tmp->key_already_exists(number);
        if (pos!=-1) { tmp->key_table[pos]->list->find(time1,year1,time2,year2); return; }

        tmp = tmp->next;

        }

        pos=tmp->key_already_exists(number);
        if (pos!=-1) { tmp->key_table[pos]->list->find(time1,year1,time2,year2); return; }
        else cout<<"Caller not found"<<endl;

}


void Key_List::indist(Call_List* l, char* number,int flag) {

	Key_Node* tmp = header;
        int pos;

        while(tmp->next!=NULL)
        {

        pos=tmp->key_already_exists(number);

        if (pos!=-1) {tmp->key_table[pos]->list->indist(l,flag);  return; }

        tmp = tmp->next;

        }


        pos=tmp->key_already_exists(number);
        if (pos!=-1) { tmp->key_table[pos]->list->indist(l,flag); }
        else cout<<"number not found"<<endl;
}

void Key_List::topdest(Country_List* l, char* number) {

        Key_Node* tmp = header;
        int pos;

        while(tmp->next!=NULL)
        {

        pos=tmp->key_already_exists(number);
        if (pos!=-1) { tmp->key_table[pos]->list->topdest(l); return; }

        tmp = tmp->next;

        }


        pos=tmp->key_already_exists(number);
        if (pos!=-1) { tmp->key_table[pos]->list->topdest(l); }
}


