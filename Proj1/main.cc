#include "header.h"
#include "heap.h"
#include <fstream>
#include <stdlib.h>
#define REC_NUMBER 8
#define BCK_SIZE 8


int main(int argc, char* argv[] ) {


	//readind parametres from command line
	char* InputFile;
	int H1_Entries;
	int H2_Entries;
	int BucketSize;


	if (argc!=9) { cout<<"You have to give 4 params: Inputfile,h1,h2,Bucketsize!!"<<endl;  return 0; }
	for(int i = 1; i<=7; i=i+2)
	{
		if (strcmp(argv[i],"-o")==0) { InputFile = new char[strlen(argv[i+1]) + 1]; strcpy(InputFile,argv[i+1]); }
		if (strcmp(argv[i],"-h1")==0) { H1_Entries = atoi(argv[i+1]); }
		if (strcmp(argv[i],"-h2")==0) { H2_Entries = atoi(argv[i+1]); }
		if (strcmp(argv[i],"-s")==0) { BucketSize = atoi(argv[i+1])/(BCK_SIZE); }

	}
	cout<<"Sizeof cdr struct: "<<sizeof(struct CDR_Record)<<endl;
	cout<<"Input file: "<<InputFile<<" Hash1: "<<H1_Entries<<" Hash2: "<<H2_Entries<<" Bucketsize: "<< BucketSize<<endl;
	////////////////////////////////////////

	Hashtable* table1 = new Hashtable(H1_Entries, BucketSize);		//data structures
	Hashtable* table2 = new Hashtable(H1_Entries, BucketSize);
	Heap*  heap = new Heap();


	char* command;
	char* cdr_uniq_id;
        char* originator_number;
        char* destination_number;
        char* date;
        char* init_time;
        int duration;
        int type;
        int tarrif;
        int fault_condition;

	char buffer[200];


	////calculating the costs from the tarrif file/////////////
	ifstream t_f ("tarrif.txt",ios::in);
	t_f.getline(buffer,sizeof(buffer));
	int a,b;
	float c;
	cost the_cost;

	for(int i=0; i<5; i++)
	{
		t_f.getline(buffer,sizeof(buffer));
		a = atoi(strtok(buffer,";"));
		b = atoi(strtok(NULL,";"));
		c = atof(strtok(NULL," "));

		if(a==0) { the_cost.cost0 = c; }
		if (a==1 && b==1) { the_cost.cost11 = c; }
		if (a==1 && b==2) { the_cost.cost12 = c; }
		if (a==2 && b==1) { the_cost.cost21 = c; }
		if (a==2 && b==2) { the_cost.cost22 = c; }
	}
	////////////////


	ifstream myfile (InputFile,ios::in);

	CDR_Record* rec;
	CDR_Record* recc;

	int count = 0;

	while(myfile.getline(buffer,sizeof(buffer)) || cin.getline(buffer,sizeof(buffer)) )
	{

	cout<< ">>> "<<buffer<<endl;

	command = strtok(buffer," ");

	if (strcmp(command,"insert")==0)
	{
	cdr_uniq_id = strtok(NULL,";");
	originator_number = strtok(NULL,";");
	destination_number = strtok(NULL,";");
	date= strtok(NULL,";");
	init_time = strtok(NULL,";");
	duration= atoi(strtok(NULL,";"));
	type = atoi(strtok(NULL,";"));
	tarrif= atoi(strtok(NULL,";"));
	fault_condition= atoi(strtok(NULL,";"));

	rec = new CDR_Record(cdr_uniq_id,originator_number,destination_number,date,init_time,duration,type,tarrif, fault_condition);
	recc = new CDR_Record(cdr_uniq_id,originator_number,destination_number,date,init_time,duration,type,tarrif, fault_condition);

	table1->Insert(originator_number,rec,REC_NUMBER);
	table2->Insert(destination_number,recc,REC_NUMBER);

	heap->Insert(originator_number,the_cost.cost_calculator(duration,type,tarrif));

	}
	else if (strcmp(command,"delete")==0)
	{
	cdr_uniq_id = strtok(NULL," ");
	char* number = strtok(NULL," ");
	table1->Delet(cdr_uniq_id,number);
	}
	else if (strcmp(command,"print")==0)
	{
		char* pch = strtok(NULL," ");
		if( strcmp(pch,"hashtable1")==0 )		table1->print_table();
		else if(strcmp(pch,"hashtable2")==0)		table2->print_table();
	}
	else if(strcmp(command,"find")==0)
	{
		char* tmp;
		char* time1;
		char* time2;
		char* year1;
		char* year2;

		originator_number = strtok(NULL," ");

		time1 = strtok(NULL," ");


		if (time1!=NULL)
		{
			tmp = strtok(NULL," ");
			time2 = strtok(NULL," ");
			if (time2==NULL)
			{
				time2 = new char[strlen(tmp)+1];
				strcpy(time2,tmp);
				table1->Find(originator_number,time1,NULL,time2,NULL);
			}
			else
			{
				year1 = new char[strlen(tmp)+1];
                        	strcpy(year1,tmp);

				year2 = strtok(NULL," ");
				table1->Find(originator_number,time1,year1,time2,year2);
			}
		}
		else 	{
			table1->Find(originator_number,NULL,NULL,NULL,NULL);
			}

	}
	else if(strcmp(command,"indist")==0)
		{
			char* caller1 = strtok(NULL," ");
			char* caller2 = strtok(NULL," ");

			Call_List* l1 = new Call_List();
			Call_List* l2 = new Call_List();

			table1->indist(l1,caller1,0);
			table2->indist(l1,caller1,1);
			table1->indist(l2,caller2,0);
			table2->indist(l2,caller2,1);

			l1->match(l2,*table1,*table2);

			delete l1;
			delete l2;
		}
	else if(strcmp(command,"lookup")==0)
        {
                char* tmp;
                char* time1;
                char* time2;
                char* year1;
                char* year2;

                char* dest_number = strtok(NULL," ");

                time1 = strtok(NULL," ");


                if (time1!=NULL)
                {
                        tmp = strtok(NULL," ");
                        time2 = strtok(NULL," ");
                        if (time2==NULL)
                        {
                                time2 = new char[strlen(tmp)+1];
                                strcpy(time2,tmp);
                                table2->Find(dest_number,time1,NULL,time2,NULL);
                        }
                        else
                        {
                                year1 = new char[strlen(tmp)+1];
                                strcpy(year1,tmp);

                                year2 = strtok(NULL," ");
				table2->Find(dest_number,time1,year1,time2,year2);
                        }
                }
                else    {
                        table2->Find(dest_number,NULL,NULL,NULL,NULL);
                        }

        }
	else if(strcmp(command,"topdest")==0)
	{
		char* caller = strtok(NULL," ");
		Country_List* l = new Country_List();
		table1->topdest(l,caller);
		l->print_max();
		delete l;
	}
	else if(strcmp(command,"top")==0)
	{
		char* pch = strtok(NULL," ");
		int percent = atoi(pch);
		heap->top(percent);
		heap->heap_initialize();

	}
	else if(strcmp(command,"exit")==0) { delete InputFile; delete table1; delete table2; delete heap; return 0; }
	else if(strcmp(command,"bye")==0)
	{
		table1->destroy();
		table2->destroy();
		heap->clear();
	}




	}

return 0;

}
