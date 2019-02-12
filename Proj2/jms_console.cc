#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio>
#include <fstream>

#define MSGSIZE 500

using namespace std;

int main(int argc, char* argv[]) {

 if ( argc!=5 && argc!=7) { cout<<"Incorrect arguments!!"<<endl; return -1; }

 int fd1,fd2,read_bytes;
 char msgbuf[MSGSIZE];
 char* fifo_in;
 char* fifo_out;
 char* op_file = NULL;
 char* path;

 for(int i=1; i<argc; i = i + 2) {
 if (strcmp(argv[i],"-w")==0) { fifo_in = new char[strlen(argv[i+1])]; strcpy(fifo_in,argv[i+1]); }
 if (strcmp(argv[i],"-r")==0) { fifo_out = new char[strlen(argv[i+1])]; strcpy(fifo_out,argv[i+1]); }
 if (strcmp(argv[i],"-o")==0) { op_file = new char[strlen(argv[i+1])]; strcpy(op_file,argv[i+1]); }
 }

 char comm_buff[MSGSIZE];

  
  ifstream myfile(op_file,ios::in);

 if (op_file!=NULL)
 cout<<"Input pipe: "<<fifo_in<<" Output pipe: "<<fifo_out<<" Operation file: "<<op_file<<endl;
 else cout<<"Input pipe: "<<fifo_in<<" Output pipe: "<<fifo_out<<" Operation file: "<<"No operation file"<<endl;


 if ( ( fd1 = open ( fifo_in , O_WRONLY) ) < 0) {				//anoigei gia grapsimo to in
 perror ( " fifo open error " ) ;
 exit (3) ;
 }

 int flag = 0;

 
 if (op_file!=NULL) {

 while(myfile.getline(msgbuf,MSGSIZE) || cin.getline(msgbuf,MSGSIZE))
 {
	cout<<"--->"<<msgbuf<<endl;

	if ( write(fd1,msgbuf,MSGSIZE )  == -1)
 	{ perror ( " Error in Writing " ) ; exit (2) ; }

	//cout<<"Console send a message!!"<<endl;

	if (flag==0)							// anoigei gia diavasma to out thn prwth fora afotou steilei mhnyma
	{								// to open ayto einai blocking wste na xreiazetai na
		if ( ( fd2 = open ( fifo_out, O_RDONLY ) ) < 0) {      // parei apanthsh - epivevaiwsh paralavhs tou mhnymatos tou apo 
 		perror ( " fifo open error " ) ;			// ton coord prin steilie to epomeno mhnyma
 		exit (1);
 		}
	flag = 1;
	}

	//cout<<"Console received a message!!"<<endl;

        if ( (read_bytes = read (fd2,msgbuf,MSGSIZE)) < 0) { perror ( " Error in Reading " ); exit (2); }
	else { 
		if(strcmp(msgbuf,"coord-exit")!=0)
		cout<<msgbuf<<endl;
		else
		{
			cout<<"Console exited"<<endl;
			delete fifo_in;
			delete fifo_out;
			delete op_file;

			exit(1);
		} 
		}
	

 }
 }
 else
 {
	while(cin.getline(msgbuf,MSGSIZE))
	{
	cout<<"--->"<<msgbuf<<endl;

	if ( write(fd1,msgbuf,MSGSIZE )  == -1)
 	{ perror ( " Error in Writing " ) ; exit (2) ; }

	//cout<<"Console send a message!!"<<endl;

	if (flag==0)							// anoigei gia diavasma to out thn prwth fora afotou steilei mhnyma
	{								// to open ayto einai blocking wste na xreiazetai na
		if ( ( fd2 = open ( fifo_out, O_RDONLY ) ) < 0) {      // parei apanthsh - epivevaiwsh paralavhs tou mhnymatos tou apo 
 		perror ( " fifo open error " ) ;			// ton coord prin steilie to epomeno mhnyma
 		exit (1);
 		}
	flag = 1;
	}

	//cout<<"Console received a message!!"<<endl;

        if ( (read_bytes = read (fd2,msgbuf,MSGSIZE)) < 0) { perror ( " Error in Reading " ); exit (2); }
	else { 
		if(strcmp(msgbuf,"coord-exit")!=0)
		cout<<msgbuf<<endl;
		else
		{
			cout<<"Console exited"<<endl;
			delete fifo_in;
			delete fifo_out;
			exit(1);
		} 
		}
	

 	}

 }
 	


 return 0;

 }
