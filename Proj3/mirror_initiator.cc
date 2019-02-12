#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <dirent.h>
#include <stdio.h>

#define BUFSZ 512

using namespace std;

void perror_exit(const char *message);

int main(int argc, char *argv[]) {
    int             port, sock, i;
    char            buf[BUFSZ];
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    struct hostent *rem;

    if (argc != 7) {
    	printf("Wrong Arguments");
       	exit(1);}

	char* hostname;
	char* input;

	for(int i=1; i<argc; i = i + 2) {
 	if (strcmp(argv[i],"-n")==0) { hostname = new char[strlen(argv[i+1]) + 1]; strcpy(hostname,argv[i+1]); }
 	if (strcmp(argv[i],"-p")==0) { port = atoi(argv[i+1]); }
 	if (strcmp(argv[i],"-s")==0) { input =    new char[strlen(argv[i+1]) + 1]; strcpy(input,argv[i+1]);  }
 	}

	cout<<"hostname: "<<hostname<<endl<<"port: "<<port<<endl<<"input: "<<input<<endl;

	
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	perror_exit("socket");
	
    if ((rem = gethostbyname(hostname)) == NULL) {	
	   herror("gethostbyname"); exit(1);
    }
  
    server.sin_family = AF_INET;       
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(port);        
  
    if (connect(sock, serverptr, sizeof(server)) < 0)
	   perror_exit("connect");
    printf("Connecting to %s port %d\n", hostname, port);

		

    strcpy(buf,input);

    char* end;

    do {

    if (write(sock, buf, sizeof(buf)) < 0)
    perror_exit("write");
            
		
    if (read(sock, buf, sizeof(buf)) < 0)
    perror_exit("read");


    end = strtok(buf,": ");
    int b_trans;
    int fil_trans;
    int dev_done;

	b_trans = atoi(strtok(NULL," "));
	fil_trans = atoi(strtok(NULL," "));
	dev_done = atoi(strtok(NULL," "));
	int m = atoi(strtok(NULL," "));
	int var = atoi(strtok(NULL," "));

	cout<<"Results--->"<<"Bytes transferred: " << b_trans <<" Files transferred: " << fil_trans << " Devices done: "<<dev_done<<endl;
	cout<<"average: "<<m<<" variance: "<<var<<endl; 
	

    }while(strcmp(end,"Results")!=0);  

    close(sock);

    cout<<"Initiator exited!"<<endl;                

return 0;
}			     

void perror_exit(const char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}
