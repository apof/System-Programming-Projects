#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define BUFSZ 512
#define LIST_SIZE 5

using namespace std;

struct Delay {
	int delay;
	int id;
	Delay() { delay = -1; id= -1; }
};

struct delay_table {

	Delay** table;

	delay_table() {
	table = new Delay*[LIST_SIZE];
	for(int i=0; i<LIST_SIZE; i++)
	table[i] = new Delay();
	}

	~delay_table() {
	for(int i=0; i<LIST_SIZE; i++)
	delete table[i];
	delete [] table;
	}

};

delay_table t;


void perror_exit ( const char * msg ) ;

int main(int argc,char* argv[]) {

    char* directory;
    int port;
    char buf[BUFSZ];

	if (argc!=5) { cout<<"You have to give 2 arguments!"<<endl; return 0; }

	for(int i=1; i<argc; i = i + 2) {
 	if (strcmp(argv[i],"-p")==0) {  port = atoi(argv[i+1]); }
 	if (strcmp(argv[i],"-d")==0) { directory = new char[strlen(argv[i+1]) + 1]; strcpy(directory,argv[i+1]); }
 	}

	cout<<"Port: "<<port<<endl<<"Directory provided: "<<directory<<endl;


        struct sockaddr_in myaddr ; 
	int c , lsock , csock ; 
	FILE* sock_fp ;
	FILE* pipe_fp ;
	char command [ BUFSZ ];

	if (( lsock = socket ( PF_INET , SOCK_STREAM , 0) ) < 0)
	perror_exit ( " socket " ) ;

	myaddr . sin_addr . s_addr = htonl ( INADDR_ANY ) ;
	myaddr . sin_port = htons ( port ) ;
	myaddr . sin_family = AF_INET ;

	int enable = 1;
	setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); 

	if ( bind ( lsock ,( struct sockaddr *) & myaddr , sizeof ( myaddr ) ) )
	perror_exit ( " bind " ) ;

	if ( listen ( lsock , 50) != 0 )
	perror_exit ( " listen " );
	printf("Listening for connections to port %d\n", port);

	while ( 1 ) { 

	if ( ( csock = accept ( lsock , NULL , NULL ) ) < 0 )
	perror_exit ( " accept " ) ;
	printf("Accepted connection\n");

	int answer;
	do { answer = read(csock,buf,BUFSZ); } while(answer <= 0) ;
	cout<<"Content server received: "<<buf<<endl;

	char* command;
	command = strtok(buf," ");

	if(strcmp(command,"LIST") == 0)
	{

	int id = atoi(strtok(NULL," "));
	int del = atoi(strtok(NULL," "));


	for(int i=0; i<LIST_SIZE; i++)
	{
		if (t.table[i]->delay == -1)
		{
			t.table[i]->delay = del;
			t.table[i]->id = id;
		}
	}

	}

	switch (fork()) {    
    	case -1:     
    	    perror("fork"); break;
    	case 0:	     

	char buf[BUFSZ];

	if(strcmp(command,"LIST") == 0)
	{

	if (( sock_fp = fdopen ( csock , "r+" ) ) == NULL )
	perror_exit ( " fdopen " ) ;

	snprintf ( command , BUFSZ , "find %s -type f" , directory ) ;

	if (( pipe_fp = popen ( command , "r" ) ) == NULL )
	perror_exit ( " popen " ) ;

	while ( ( c = getc ( pipe_fp ) ) != EOF )
	putc (c , sock_fp ) ;

	pclose ( pipe_fp ) ;
	fclose ( sock_fp ) ;

	}
	else 
	{

	char* Dir = strtok(NULL," ");
	int id = atoi(strtok(NULL," "));

	for(int i=0; i<LIST_SIZE; i++)
	{
		if (t.table[i]->id == id)
		{
		cout<<"Sleeping for: "<<t.table[i]->delay<<" secs before fetch"<<endl;
		sleep(t.table[i]->delay);
		break;
		}
	}

	
	if (( sock_fp = fdopen ( csock , "r+" ) ) == NULL )
	perror_exit ( " fdopen " ) ;

	FILE* fp = fopen(Dir,"r");
	
	while ( fread(buf,1,BUFSZ,fp) > 0)
	{
	fwrite(buf,1,BUFSZ,sock_fp);
	}

	fclose ( fp ) ;
	fclose ( sock_fp ) ;

	}

	}

	close(csock);

	}

	return 0;
}


	void perror_exit ( const char * message )
	{
	perror ( message ) ;
	exit ( EXIT_FAILURE ) ;
	}

