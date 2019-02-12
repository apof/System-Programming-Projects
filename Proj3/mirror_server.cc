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
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#define perror2(s, e) fprintf(stderr, "%s: %s\n", s, strerror(e))

#define BUFSZ 512
#define INPUT 10
#define POOL_SIZE 4

using namespace std;

class Mutex
{
    friend class CondVar;
    pthread_mutex_t  m_mutex;
 
  public:
    Mutex() { pthread_mutex_init(&m_mutex, NULL); }
    virtual ~Mutex() { 
	    pthread_mutex_unlock(&m_mutex); 
	    pthread_mutex_destroy(&m_mutex); 
	}
 
    int lock() { return  pthread_mutex_lock(&m_mutex); }
    int trylock() { return  pthread_mutex_trylock(&m_mutex); }
    int unlock() { return  pthread_mutex_unlock(&m_mutex); }   
};

class CondVar
{
    pthread_cond_t  m_cond;
    Mutex&          m_lock;
 
  public:
    CondVar(Mutex& mutex) : m_lock(mutex) { pthread_cond_init(&m_cond, NULL); }
    virtual ~CondVar() { pthread_cond_destroy(&m_cond); }
 
    int wait() { return  pthread_cond_wait(&m_cond, &(m_lock.m_mutex)); }
    int signal() { return pthread_cond_signal(&m_cond); } 
    int broadcast() { return pthread_cond_broadcast(&m_cond); } 
 
  private:
    CondVar();
};

void perror_exit(const char *message);
void sigchld_handler (int sig);
void *manager(void*); 
void *worker(void*);
int write_all (int, char* ,void*,size_t);

Mutex mutex;
CondVar write_cond(mutex);
CondVar read_cond(mutex);
int readers = 0;
bool writer = false;

struct pool_data {
	char* name;
	int con_server_id;
	int port;
	char* host;
	pool_data() { name = NULL; host = NULL; }
	~pool_data() { if (name!=NULL) delete name; if (host!=NULL) delete host;  }
	void add(char* nam,int id,int p,char* ho) { 
	host = new char[strlen(ho) + 1]; strcpy(host,ho);
	name = new char[strlen(nam) + 1]; strcpy(name,nam); 
	con_server_id = id; port = p; }
	void del() { if (name!=NULL) delete name; if (host!=NULL) delete host; }
	void print() { if (name!=NULL) cout<<name<<" "<<port<<" "<<con_server_id<<" "<<host<<endl; else cout<<"Empty!"<<endl; }
	
};


struct pool_t {
	pool_data** data;
	int start;
	int end;
	int count;

	pool_t() {
		start = 0;
		end = -1;
		count = 0;
		data = new pool_data*[POOL_SIZE];
		for(int i = 0; i<POOL_SIZE; i++)
		data[i] = new pool_data();
	}

	~pool_t() {
		for(int i = 0; i<POOL_SIZE; i++)
		delete data[i];
		delete [] data;
	}

	void print() { 
		for(int i = 0; i<POOL_SIZE; i++)
		data[i]->print();
		cout<<start<<" "<<end<<" "<<count<<endl;
	} 


};

pool_t pool ;

struct input {

	char* ContAddress;
	char* DirOrFile;
	int Port;
	int Delay;
	int manager_id;

	input(char* CA,char* DF,int P,int D,int id) {
	ContAddress = new char[strlen(CA) + 1]; strcpy(ContAddress,CA);
	DirOrFile = new char[strlen(DF) + 1];   strcpy(DirOrFile,DF);
	Port = P;
	Delay = D; 
	manager_id = id;
	}

	~input() {
	delete ContAddress;
	delete DirOrFile;
	}

	void print() { cout<<ContAddress<<" "<<DirOrFile<<" "<<Port<<" "<<Delay<<endl; }

};

char save_directory[BUFSZ];

int* request;
int req_num;

int filesTransferred;
int numDevicesDone;

int* bytes_per_server;

int main(int argc, char *argv[]) {
    int             port, sock, newsock;
    struct sockaddr_in server, client;
    socklen_t clientlen;
    struct sockaddr *serverptr=(struct sockaddr *)&server;
    struct sockaddr *clientptr=(struct sockaddr *)&client;
    struct hostent *rem;

    char* save_dir;
    int workers_num;

	filesTransferred = 0;	

    for(int i=1; i<argc; i = i + 2) {
 	if (strcmp(argv[i],"-p")==0) { port = atoi(argv[i+1]); }
 	if (strcmp(argv[i],"-w")==0) { workers_num = atoi(argv[i+1]); }
 	if (strcmp(argv[i],"-m")==0) { save_dir =    new char[strlen(argv[i+1]) + 1]; strcpy(save_dir,argv[i+1]);  }
 	}

	cout<<"port: "<<port<<endl<<"workers_num: "<<workers_num<<endl<<"save directory: "<<save_dir<<endl;

	strcpy(save_directory,save_dir);

	struct stat st = {0};

	if (stat(save_directory, &st) == -1) {
    	mkdir(save_directory, 0700);
	}
    
    signal(SIGCHLD, sigchld_handler);
    
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    server.sin_family = AF_INET;       
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

	int enable = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));     
    
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("bind");
   
    if (listen(sock, 5) < 0) perror_exit("listen");
    printf("Listening for connections to port %d\n", port);
    while (1) {

    	if ((newsock = accept(sock, clientptr, &clientlen)) < 0) perror_exit("accept");
    	printf("Accepted connection\n");

    	switch (fork()) {    
    	case -1:     
    	    perror("fork"); break;
    	case 0:	     
    	    close(sock);
	    char buf[BUFSZ];

            if (read(newsock, buf, sizeof(buf)) > 0)  
	    cout<<"server received: "<<buf<<endl;

	    input** input_Array = new input*[INPUT];
	    for ( int init = 0; init<INPUT; init++ )
		input_Array[init] = NULL;

		char* Adr;
		char* save_dir;
		int delay;
		int Port;
		int table_pos = 0;

		Adr = strtok(buf,":");
		while(Adr!=NULL)
		{
		 Port = atoi(strtok(NULL,":"));
		 save_dir = strtok(NULL,":");
		 delay = atoi(strtok(NULL,","));

		input_Array[table_pos] = new input(Adr,save_dir,Port,delay,table_pos + 1);
		table_pos++;
		
		Adr = strtok(NULL,":");
		}

		request = new int[table_pos];
		for(int i=0; i<table_pos; i++)
		request[i] = -1;

		bytes_per_server = new int[table_pos];
		for(int i=0; i<table_pos; i++)
		bytes_per_server[i] = 0;
		

		cout<<"Managers to create: "<< table_pos<<endl;

		req_num = table_pos;

		pthread_t *m_tids;
		pthread_t *w_tids;
		int err;

		m_tids =new pthread_t[table_pos - 1];
		w_tids = new pthread_t[workers_num];

		for (int i=0 ; i<table_pos ; i++) {
       		if (err = pthread_create(m_tids+i, NULL, manager, (void *)input_Array[i] )) {
       		perror2("pthread_create", err); exit(1); } }

		for (int i=0; i<workers_num; i++) {
       		if (err = pthread_create(w_tids+i, NULL, worker,NULL)) {
       		perror2("pthread_create", err); exit(1); } }

    		for (int i=0 ; i<table_pos ; i++)
       		if (err = pthread_join(*(m_tids+i), NULL)) {
           	perror2("pthread_join", err); exit(1); }

		
    		for (int i=0 ; i<workers_num ; i++)
       		if (err = pthread_join(*(w_tids+i), NULL)) {
           	perror2("pthread_join", err); exit(1); }

		delete m_tids;
		delete w_tids;
		delete request;
		for(int j = 0; j<table_pos; j++)
		delete input_Array[j];
		delete [] input_Array;

		cout<<"Bytes per server: "<<endl;
		long int bytesTransferred = 0;
		for(int i=0; i<req_num; i++)
		{
		cout<<"Content server "<<i + 1<<" "<<bytes_per_server[i]<<" bytes"<<endl;
		bytesTransferred += bytes_per_server[i];
		}

		unsigned int var = 0;
		int m = bytesTransferred/req_num;
		for(int i=0; i<req_num; i++)
		{
		var += (bytes_per_server[i] - m)*(bytes_per_server[i] - m);
		}
		var = var/req_num;
		

		cout<<"bytesTransferred: "<<bytesTransferred<<endl;
		cout<<"filesTransferred: "<<filesTransferred<<endl;
		cout<<"numDevicesDone: "<<numDevicesDone<<endl;

		char final_req[100];
		sprintf(final_req,"Results: %ld %d %d %d %d",bytesTransferred,filesTransferred,numDevicesDone,m,var );
	    
    	    if(write(newsock, final_req , sizeof(final_req)) < 0)
    	    perror_exit("write");

            printf("Closing connection.\n");
            close(newsock);	 
    	    exit(0);
    	}
    	close(newsock);
    }

return 0;
}



/* Wait for all dead child processes */
void sigchld_handler (int sig) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void perror_exit(const char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void *manager(void *argp){
    
	struct input* in = (input* )argp;
	cout<<"Manager to handle: "<<" "; in->print();

	struct sockaddr_in servadd ; 
	struct hostent * hp ;
	int sock , n_read ;
	char buf [BUFSZ];


        if (( sock = socket ( PF_INET , SOCK_STREAM , 0) ) == -1 )
	perror_exit ( " socket " ) ;

	if (( hp = gethostbyname ( in->ContAddress)) == NULL ) {
	herror ( " gethostbyname " ) ; exit (1) ;}
	memcpy (&servadd.sin_addr , hp->h_addr , hp->h_length ) ;
	servadd.sin_port = htons ( in->Port ) ; 
	servadd.sin_family = AF_INET ;

	if ( connect ( sock , ( struct sockaddr *) & servadd , sizeof ( servadd ) ) !=0)
	perror_exit ( " connect " ) ;

	char out_file[20];
	sprintf(out_file,"manager%dfile",in->manager_id);
	int fd = creat(out_file,O_RDWR | O_CREAT | S_IRWXU|S_IWUSR|S_IRGRP|S_IROTH);

	sprintf(buf,"LIST %d %d",in->manager_id,in->Delay);
	write(sock,buf,BUFSZ);

	while ( ( n_read = read ( sock , buf , BUFSZ) ) > 0 )
	if ( write_all ( fd, in->DirOrFile, buf , n_read ) < n_read )
	perror_exit ( " fwrite " ) ;

	int numLines = 0;
	ifstream myfile_tmp(out_file,ios::in);
	while(myfile_tmp.getline(buf,BUFSZ))
	numLines++;

	request[in->manager_id - 1] = numLines;

	ifstream myfile(out_file,ios::in);
	int found_flag = 0;
	while(myfile.getline(buf,BUFSZ))
	{
		if(strstr(buf,in->DirOrFile)!=NULL)
		{  
			found_flag = 1;

			mutex.lock();
			while (readers > 0 || writer || pool.count >= POOL_SIZE )
			write_cond.wait();
			writer = true ;
			
			pool.end = ( pool.end + 1) % POOL_SIZE ;
			cout<<buf<<" placed on buffer "<<endl;
			pool.data[pool.end]->add(buf,in->manager_id,in->Port,in->ContAddress) ;
			pool.count ++;

			mutex.unlock();
		
			mutex.lock() ;
			writer = false ;
			read_cond.signal() ;
			write_cond.signal() ;
			mutex.unlock() ; 
		}
	} 
	if (found_flag == 0) { cout<<in->DirOrFile<<" not found in "<<in->ContAddress<<" by manager "<<in->manager_id<<endl; } 

	close ( sock ) ;
		
    	pthread_exit(NULL); 
}

void *worker(void *argp) {


	while(1) {

	mutex.lock();
	while (writer || readers > 0 || pool.count <= 0 )
	{
	
	numDevicesDone = 0;
	int flg = 0;
	for(int i = 0; i<req_num; i++ )
	{
	if(request[i]!=0) flg++; 
	else numDevicesDone++;
	}
	if (flg==0) { cout<<"Worker ready to exit!"<<endl; read_cond.broadcast(); mutex.unlock(); pthread_exit(NULL); } 
	read_cond.wait();
	}
	readers++;
	mutex.unlock();

	mutex.lock();
	readers--;

	int len = 0;
	char final_buf[BUFSZ];
	char* pch;
	char final_path[BUFSZ];

	char start_dir[BUFSZ];
	sprintf(start_dir,"%s/%s_%d",save_directory,pool.data [ pool.start ]->host,pool.data [ pool.start ]->port);

	struct stat st = {0};

	if (stat(start_dir, &st) == -1)
    	mkdir(start_dir, 0700);
	

	pch = strtok(pool.data [ pool.start ]->name,"/");
	while(pch!=NULL)
	{
		len += sprintf(final_buf+len,"%s/", pch);
		pch = strtok(NULL,"/");

		sprintf(final_path,"%s/%s",start_dir,final_buf);

		struct stat st = {0};

		if (stat(final_path, &st) == -1)
    		mkdir(final_path, 0700);
	}

	rmdir(final_path);
	int pos = 0;
	while(final_path[pos]!='\0') pos++;
	final_path[--pos]='\0';
	int creat_fd = creat(final_path,O_RDWR | O_CREAT | S_IRWXU|S_IWUSR|S_IRGRP|S_IROTH);

	struct sockaddr_in servadd ; 
	struct hostent * hp ;
	int sock , n_read ;
	char buf [BUFSZ];

        if (( sock = socket ( PF_INET , SOCK_STREAM , 0) ) == -1 )
	perror_exit ( " socket " ) ;

	if (( hp = gethostbyname ( pool.data [ pool.start ]->host)) == NULL ) {
	herror ( " gethostbyname " ) ; exit (1) ;}
	memcpy (&servadd.sin_addr , hp->h_addr , hp->h_length ) ;
	servadd.sin_port = htons ( pool.data [ pool.start ]->port) ; 
	servadd.sin_family = AF_INET ;

	if ( connect ( sock , ( struct sockaddr *) & servadd , sizeof ( servadd ) ) !=0)
	perror_exit ( " connect " ) ;

	pos = 0;
	while(final_buf[pos]!='\0') pos++;
	final_buf[--pos]='\0';
	
	sprintf(buf,"FETCH %s %d",final_buf,pool.data [ pool.start ]->con_server_id);
	write(sock,buf,BUFSZ);

	while ( ( n_read = read ( sock , buf , BUFSZ) ) > 0 )
	{

	bytes_per_server[pool.data [ pool.start ]->con_server_id - 1 ] += n_read;

	if ( write_all ( creat_fd, NULL, buf , n_read ) < n_read )
	perror_exit ( " fwrite " ) ;
	}

	filesTransferred++;

	request[pool.data [ pool.start ]->con_server_id - 1 ]--;

	pool.start = ( pool.start + 1) % POOL_SIZE ;
	pool.count--;

	write_cond.signal();
	read_cond.signal();

	cout<<final_buf<<" recreated and removed from buffer"<<endl;
	mutex.unlock();
 

	}    
}





int write_all ( int fd, char* Dir , void * buff , const size_t size ) {
int sent , n;
for ( sent = 0; sent < size ; sent += n ) {
if (( n = write ( fd, (char*)buff + sent , size - sent ) ) == -1)
return -1;
}
return sent ;
}
