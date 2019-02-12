#include "header.h"

void CDR_List::Insert_List(CDR_Record* record)
	{
		if (header==NULL)
		{
		header = new Record_Node(rec_num);
		header->Insert_Record(record);
		}
		else
		{
		if (header->Free_Space()) { header->Insert_Record(record);  }
		else
		{
			Record_Node* tmp = header;
			while(tmp->next!=NULL) tmp=tmp->next;
			if (tmp->Free_Space()) tmp->Insert_Record(record);
			else { tmp->next = new Record_Node(rec_num); (tmp->next)->Insert_Record(record); }
		}
		}
	}

CDR_List::~CDR_List()
	{
		Record_Node* tmp = header;
		Record_Node* tmp1;
		while(tmp!=NULL)
		{
			tmp1=tmp->next;
			delete tmp;
			tmp = tmp1;
		}
	}

void CDR_List::Print_List()
	{
		Record_Node* tmp = header;
		while(tmp!=NULL) { tmp->Print_Table(); tmp=tmp->next; }
	}


int CDR_List::delete_record(char* id) {

	Record_Node* tmp = header;
	Record_Node* tmp1 = header;
	int pos;


	if (tmp!=NULL)
	{
	while(tmp->record_exists(id)==-1 && tmp!=NULL)
	{
		tmp1=tmp;
		tmp=tmp->next;
		if(tmp == NULL)	break;
	}

	if (tmp!=NULL)
	{
        pos = tmp->record_exists(id); 
	delete tmp->Rec_Table[pos]; tmp->Rec_Table[pos]=NULL;

	if (tmp->is_empty())
	{
		if(tmp1==tmp)
		header=tmp1->next;
		else
		tmp1->next = tmp->next;
		delete tmp;
		return 1;
	}
	}
	}
	else cout<<"not found"<<endl;
	return 0;
}


void CDR_List::find(char* time1, char* year1, char* time2, char* year2) {

	if (time1==NULL)
	{
		Record_Node* tmp = header;
                int pos;

                while(tmp->next!=NULL)
                {
                        tmp->record_find();
                        tmp=tmp->next;
                }

                tmp->record_find();
	}
	else if (year1==NULL)
	{
		Record_Node* tmp = header;

        	while(tmp->next!=NULL)
        	{
			tmp->record_find_time(time1,time2);
			tmp=tmp->next;
        	}

		tmp->record_find_time(time1,time2);
	}
	else
	{

		Record_Node* tmp = header;

                while(tmp->next!=NULL)
                {
                        tmp->record_find_all(time1,year1,time2,year2);
                        tmp=tmp->next;
                }

                tmp->record_find_all(time1,year1,time2,year2);

	}


}


int year_compare(char* str1, char* str2)
{
	char buff1[30];
	char day1[3];
	char month1[3];
	char year1[5];

	char buff2[30];
        char day2[3];
        char month2[3];
        char year2[5];

	strcpy(buff1,str1);
	strcpy(buff2,str2);

	day1[0] = buff1[0]; day1[1] = buff1[1]; day1[2]='\0';
	month1[0] = buff1[2]; month1[1] = buff1[3]; month1[2] = '\0';
	year1[0] = buff1[4]; year1[1] = buff1[5]; year1[2] = buff1[6]; year1[3] = buff1[7]; year1[4]='\0';

	day2[0] = buff2[0]; day2[1] = buff2[1]; day2[2]='\0';
        month2[0] = buff2[2]; month2[1] = buff2[3]; month2[2] = '\0';
        year2[0] = buff2[4]; year2[1] = buff2[5]; year2[2] = buff2[6]; year2[3] = buff2[7]; year2[4]='\0';

	if (atoi(year1) > atoi(year2) ) return 1;                                                             //str1 pio prosfato
	if (atoi(year1) == atoi(year2) && atoi(month1) > atoi(month2) ) return 1;
	if (atoi(year1) == atoi(year2) && atoi(month1) == atoi(month2) && atoi(day1) > atoi(day2) ) return 1;

	return 2;												//str2 pio prosfato


}

void CDR_List::indist(Call_List* l, int flag) {

	Record_Node* tmp = header;

        while(tmp->next!=NULL)
        {


	for(int i=0; i<rec_num; i++)
	{

		if (tmp->Rec_Table[i]!=NULL)
		{

			if (flag==1) { l->Insert(tmp->Rec_Table[i]->originator_number); }
			else { l->Insert(tmp->Rec_Table[i]->destination_number); }
		}
	}

        tmp=tmp->next;

	}

        for(int i=0; i<rec_num; i++)
        {
                if (tmp->Rec_Table[i]!=NULL)
                {
                        if (flag==1) { l->Insert(tmp->Rec_Table[i]->originator_number); }
                        else { l->Insert(tmp->Rec_Table[i]->destination_number); }
                }

        }


}

void CDR_List::topdest(Country_List* l) {

        Record_Node* tmp = header;

        while(tmp->next!=NULL)
        {


        for(int i=0; i<rec_num; i++)
        {
        if (tmp->Rec_Table[i]!=NULL) { l->Insert(tmp->Rec_Table[i]->destination_number); }
        }

        tmp=tmp->next;

        }

        for(int i=0; i<rec_num; i++)
        {
 	if (tmp->Rec_Table[i]!=NULL)  {  l->Insert(tmp->Rec_Table[i]->destination_number); }
        }


}

