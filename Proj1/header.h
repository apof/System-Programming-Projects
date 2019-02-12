#ifndef HEAD
#define HEAD

#include <string.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>

using namespace std;

class call_node;
class country_node;
class Call_List;
class Country_List;

//////useful funcs///////


int year_compare(char*,char*);              	//epistrefei poia hmeromhnia einai megalyterh 1 an str1>str2 2 an str1<str2

///////////////////////////////////////////////////////////////////////////////

struct cost {

		float cost0;
		float cost11;
		float cost12;
		float cost21;
		float cost22;

		float cost_calculator(int duration,int type,int tarrif)             //ypologizei kai epistrefei to kostos ths epikoinwnias gia ena record
		{
			if (type==0) return cost0;
			if(type==1 && tarrif==1) return duration*cost11;
			if(type==1 && tarrif==2) return duration*cost12;
			if(type==2 && tarrif==1) return duration*cost21;
			if(type==2 && tarrif==2) return duration*cost22;
		}

		void print() { cout<<cost0<<" "<<cost11<<" "<<cost12<<" "<<cost21<<" "<<cost22<<endl; }

};

////////////////////////////////////////// CDR List //////////////////////////////////////////////

struct CDR_Record {
	char* cdr_uniq_id;
	char* originator_number;
	char* destination_number;
	char* date;
	char* init_time;
	int duration;
	int type;
	int tarrif;
	int fault_condition;

	CDR_Record(char* id,char* origin_number,char* dest_number,char* dat,char* in_tim,int dur,int typ,int tarri,int fault_cond)
	{
		cdr_uniq_id = new char[strlen(id)+1];
		strcpy(cdr_uniq_id,id);
		originator_number = new char[strlen(origin_number)+1];
                strcpy(originator_number,origin_number);
		destination_number = new char[strlen(dest_number)+1];
                strcpy(destination_number,dest_number);
        	date = new char[strlen(dat)+1];
		strcpy(date,dat);
        	init_time = new char[strlen(in_tim)+1];
		strcpy(init_time,in_tim);
        	duration=dur;
        	type=typ;
		tarrif=tarri;
        	fault_condition=fault_cond;
	}

	~CDR_Record() { delete (init_time); delete (cdr_uniq_id); delete (date);  delete (originator_number); delete (destination_number); }

	void Print_Record()
	{
	cout<<cdr_uniq_id<<" "<<originator_number<<" "<<destination_number<<" "<<date<<" "<<init_time<<" "<<duration<<" "<<type<<" "<<tarrif<<" "<<fault_condition<<endl;
	}
};

struct Record_Node{

	CDR_Record** Rec_Table;
	Record_Node* next;
	int records_number;

	Record_Node(int number_of_records)
	{
		Rec_Table = new CDR_Record*[number_of_records];			//initializing record_table
		for(int i=0; i<number_of_records; i++)
		Rec_Table[i] = NULL;

		next = NULL;

		records_number = number_of_records;
	}


	~Record_Node()
	{
		for(int i=0; i<records_number; i++)
		{
			if (Rec_Table[i] != NULL) { delete Rec_Table[i];  Rec_Table[i]=NULL; }
		}
		delete [] Rec_Table;
	}

	void Print_Table()
	{
		for(int i=0; i<records_number; i++)
		{
		if (Rec_Table[i]!=NULL)	{ Rec_Table[i]->Print_Record();  }
		}

	}

	int Free_Space()
	{
		for(int i=0; i<records_number; i++)
                {
                if (Rec_Table[i]==NULL) return 1;
                }

		return 0;
	}

	void Insert_Record(CDR_Record* record)
	{
		for(int i=0; i<records_number; i++)
                {
                if (Rec_Table[i]==NULL) { Rec_Table[i] = record; return; }
                }
	}

	int record_exists(char* id)
	{
		for(int i=0; i<records_number; i++)
                {
		if (Rec_Table[i]!=NULL) { if (strcmp(Rec_Table[i]->cdr_uniq_id,id)==0) return i; }
                }

                return -1;
	}

	int is_empty() { 
		int count=0; 
		for(int i=0; i<records_number; i++) 	{ if (Rec_Table[i]==NULL) count++; }
		if (count==records_number) return 1; 
		else return 0; 
	}


	void record_find()
        {
                for(int i=0; i<records_number; i++)
                {
                if (Rec_Table[i]!=NULL) { Rec_Table[i]->Print_Record(); }
                }


        }


	void record_find_time(char* time1, char* time2)
	{
		for(int i=0; i<records_number; i++)
                {

		if (Rec_Table[i]!=NULL)
		{
                	if(strcmp(time1,Rec_Table[i]->init_time)<=0 && strcmp(time2,Rec_Table[i]->init_time)>=0 )
			Rec_Table[i]->Print_Record();
		}

                }


	}

	void record_find_all(char* time1, char* year1, char* time2, char* year2)
        {
		for(int i=0; i<records_number; i++)
                {

                if (Rec_Table[i]!=NULL)
                {
			if (year_compare(Rec_Table[i]->date,year1) == 1 && year_compare(Rec_Table[i]->date,year2) == 2 )
			{
                        	if(strcmp(time1,Rec_Table[i]->init_time)<=0 && strcmp(time2,Rec_Table[i]->init_time)>=0 )
                        	Rec_Table[i]->Print_Record();
			}
                }

                }

        }


};

class CDR_List {

	private:
	Record_Node* header;
	int rec_num;
	public:
	CDR_List(int rec_n) { header=NULL; rec_num = rec_n; }
	~CDR_List();
	void Insert_List(CDR_Record*);
	void Print_List();
	int delete_record(char*);
	void find(char*,char*,char*,char*);
	void indist(Call_List*,int);
	void topdest(Country_List*);
};

///////////////////////////////Key List////////////////////////////////////////////////

struct Key {
	char* key;
	CDR_List * list;
	Key(int rec_num)  { list = new CDR_List(rec_num); key=NULL; }
	~Key() { delete list; if (key!=NULL) delete key; }
	void Print_Key() { cout<< "--->" <<key<<endl; list->Print_List(); }
};

struct Key_Node {
	Key** key_table;
	Key_Node* next;
	int key_number;

	Key_Node(int key_num, int rec_num)
	{

	key_number = key_num;

	key_table = new Key*[key_number];
	for(int i=0; i<key_number; i++)
	key_table[i] = new Key(rec_num);

	next = NULL;
	}

	~Key_Node()
	{
	for(int i=0; i<key_number; i++)
	{
		if (key_table[i]!=NULL)
		{
			delete key_table[i];
			key_table[i] = NULL;
		}
	}
	delete[] key_table;
	}

	void Print_key_table()
	{
		for(int i=0; i<key_number; i++)
		{
		if(key_table[i]->key!=NULL) { key_table[i]->Print_Key(); } else cout<<"Empty"<<endl;
		}
		cout<<"----------------------------------------------------------------------------"<<endl;
	}

	int Free_Space() { for(int i=0; i<key_number; i++) if (key_table[i]->key==NULL) { return i;} return -1; }

	int is_empty() { 
		int count=0; 
		for(int i=0; i<key_number; i++) 	{ if (key_table[i]->key==NULL) count++; }
		if (count==key_number) return 1; 
		else return 0; 
	}

	int  key_already_exists(char* k)
	{
		for(int i=0; i<key_number; i++)
		{
			if (key_table[i]->key!=NULL)
			{
				if (strcmp(key_table[i]->key,k)==0) { return i; }
			}
		}
		return -1;
	}

	void Insert_key(char* k, CDR_Record* rec,int pos) {
	if (key_table[pos]->key == NULL)
	{
		key_table[pos]->key = new char[strlen(k)+1];
		strcpy(key_table[pos]->key,k);
	}
	key_table[pos]->list->Insert_List(rec);
	}

};

class Key_List {
	private:
	Key_Node* header;
	int key_number;
	public:
	Key_List(int key_n) { key_number = key_n; header=NULL; }
	~Key_List();
	void Insert_List(char*,CDR_Record*,int);
	void Print_List();
	void delet(char*,char*);
	void find(char*,char*,char*,char*,char*);
	void indist(Call_List*,char*,int);
	void topdest(Country_List*,char*);
};

//////////////////////////// Hashtable ///////////////////////////////////

class Hashtable {
	private:
	Key_List **table;
	int number_of_buckets;
	int key_number;
	public:
	Hashtable(int buck_num,int key_num)
	{
		key_number = key_num;
		number_of_buckets = buck_num;
		table = new Key_List*[buck_num];
		for (int i=0; i<buck_num; i++)
		table[i] = NULL;
	}

	void destroy()
	{
		for(int i=0; i<number_of_buckets; i++) { if (table[i]!=NULL) { delete table[i]; table[i]=NULL; } }
	}

	~ Hashtable() { destroy(); delete [] table;  }

	int Hash_Func(char* key) {
	int sum = 0;
	for (unsigned int i = 0; i < strlen(key); i++) sum += key[i];
        return sum % number_of_buckets;
	}

	void Insert(char* k,CDR_Record* rec,int rec_num)
	{
		if (table[Hash_Func(k)]==NULL)  table[Hash_Func(k)] = new Key_List(key_number);

		table[Hash_Func(k)]->Insert_List(k,rec,rec_num);
	}

	void print_table()
	{
		cout<<endl;
		cout<<endl;
		cout<<"HASTABLE PRINT"<<endl;
		for(int i=0; i<number_of_buckets; i++)
		{
			cout<<"###### BUCKET " << i <<" ########"<<endl;
			if(table[i]!=NULL) table[i]->Print_List();
		}
	}

	void Delet(char* id,char* key) {  if (table[Hash_Func(key)]!=NULL) table[Hash_Func(key)]->delet(id,key);  }

	void Find(char* number,char* a,char* b,char* c,char* d)
	{if (table[Hash_Func(number)]!=NULL)  table[Hash_Func(number)]->find(number,a,b,c,d); else cout<<"number not found!"<<endl; }

	void   indist(Call_List* l,char* number,int flag)
	{if (table[Hash_Func(number)]!=NULL) table[Hash_Func(number)]->indist(l,number,flag); else cout<<"number not found!"<<endl; }

	void topdest(Country_List* l, char* number)
	{ if (table[Hash_Func(number)]!=NULL) table[Hash_Func(number)]->topdest(l,number); }

};

/////////////////////////////////// CALL LIST //////////////////////////////////////////

struct call_node {
                char* number;
                call_node* next;
                call_node(char* c) { next = NULL; number = new char[strlen(c) + 1]; strcpy(number,c); }
                ~call_node() { delete number; }
        };


class Call_List {

	private:
	call_node* header;
	int size;

	public:

	Call_List() { header = NULL; size=0; }

	void Insert(char* num)
		{

		call_node* tmp = header;

		if (header==NULL) { header = new call_node(num); size++; }
		else
		{
		header = new call_node(num);
		header->next = tmp;
		size++;
		}

		}

	~Call_List()
		{

		call_node* tmp = header;
		call_node* tmp2;
		while(tmp!=NULL)
		{
		tmp2=tmp->next;
		delete tmp;
		tmp = tmp2;
		}

		header = NULL;

		}

	call_node* return_header() { return header; }

	void match(Call_List* list1, Hashtable& H1, Hashtable& H2)
	{
		if (header==NULL) return;

		Call_List* list3 = new Call_List();

		call_node* tmp = header;
		while(tmp!=NULL)
		{
			char* num = new char[ strlen(tmp->number)+1];
			strcpy(num,tmp->number);

			call_node* temp = list1->return_header();

			if(temp==NULL) return;
				while(temp!=NULL)
				{
					if (strcmp(num,temp->number)==0) list3->Insert(num);
					temp = temp->next;
				}

			tmp=tmp->next;

			delete num;
			
		}

		
		if(list3->get_size()==1) { list3->print(); delete list3; return; }

		
			call_node* t = list3->return_header();

			while(t!=NULL)
			{
				Call_List* l4 = new Call_List();

				H1.indist(l4,t->number,0);
				H2.indist(l4,t->number,1);

				call_node* a = l4->return_header();
				int flag = 0;
				while(a!=NULL)
				{
					if (strcmp(t->number,a->number)==0) flag = 1;
					a=a->next;
						
				}
				if (flag==0) cout<<t->number<<endl;
				
				delete l4;

				t=t->next;
		
				
			}
		
		
	}

	void print(){
		call_node* tmp = header;
                while(tmp!=NULL)
                {
			cout<< tmp->number<<" ";
			tmp=tmp->next;
		}
		cout<<endl;
		}

	int get_size() { return size; }

};

////////////////////////////////////// COUNTRY LIST ///////////////////////////////////////

struct country_node {
                char* code;
                country_node* next;
		int count;

                country_node(char* number)
		{
		next=NULL;
		char* c = strtok(number,"-");
		code = new char[strlen(c) + 1];
		strcpy(code,c);
		count=1;
		}

                ~country_node() { delete code; }
};

class Country_List {
	private:
		country_node* header;
		int size;
		int max_number;
	public:
		Country_List() { header = NULL; size=0; max_number=0; }

		 ~Country_List()
	                {

        	        country_node* tmp = header;
                	country_node* tmp2;
                	while(tmp!=NULL)
                	{
                	tmp2=tmp->next;
                	delete tmp;
                	tmp = tmp2;
                	}

                	}

		void Insert(char* num)
			{

     			if (header==NULL) { header = new country_node(num); size++; max_number++; }
     			else
      			{
				country_node *tmp = header;

				char* pch;
				pch = strtok(num,"-");

	       			while(tmp->next!=NULL && strcmp(tmp->code,pch)!=0)
       				tmp = tmp->next;
       				if (tmp->next==NULL &&  strcmp(tmp->code,pch)!=0)
         			{ tmp->next = new country_node(num); }
       				else
				{
         			tmp->count++;
				if (tmp->count > max_number) max_number = tmp->count;
				}

       				size++;
      			}

			}

		void print_max()
		{
			country_node *tmp = header;
			while(tmp!=NULL)
			{
				if(max_number == tmp->count)
				cout<< tmp->code << "--" << tmp->count<<endl;
				tmp=tmp->next;
			}
		}


};



#endif
