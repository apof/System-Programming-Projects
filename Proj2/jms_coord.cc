#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <cstdio> 
#include <signal.h>
#include <time.h>

#define MSGSIZE 500
#define MAX_POOLS 100

using namespace std;

struct pool_info {

	int fd_in;
	int fd_out;
	int status;
	int pid;
	long int *exit_times;
	int max_jobs;
	pool_info(int max) { max_jobs = max; status = 1; exit_times = new long int[max]; }
	~pool_info() { delete exit_times; }

	void print() { cout<<"***> "<<fd_in<<" "<<fd_out<<" "<<status<<" "<<pid<<endl; }
	

};


int main(int argc, char* argv[]) {

 if ( argc!=9 ) { cout<<"Incorrect arguments!!"<<endl; return -1; }

 char* path;
 char* fifo_in;
 char* fifo_out;
 int job_number;
 int read_bytes,nwrite;
 int fd1,fd2;

 char msgbuf[MSGSIZE];

 for(int i=1; i<argc; i = i + 2) {
 if (strcmp(argv[i],"-l")==0) { path = new char[strlen(argv[i+1])]; strcpy(path,argv[i+1]); }
 if (strcmp(argv[i],"-n")==0) job_number=atoi(argv[i+1]);
 if (strcmp(argv[i],"-w")==0) { fifo_out = new char[strlen(argv[i+1])]; strcpy(fifo_out,argv[i+1]); }
 if (strcmp(argv[i],"-r")==0) { fifo_in = new char[strlen(argv[i+1])]; strcpy(fifo_in,argv[i+1]); }
 }

 cout<<"Path: "<<path<<" Job number: "<<job_number<<endl<<"InPipe: "<<fifo_in<<" Outpipe: "<<fifo_out<<endl;

 struct stat st = {0}; //dhmiourgia tou directory pou tha mpoun ta output files kai ta pipes ama aftos den yparxei

 if (stat(path, &st) == -1) {
    mkdir(path, 0700);
 }

 //dhmiourgia tou pipe in
 if (mkfifo(fifo_in,0666) == -1 ) {
 if ( errno != EEXIST ) {
 perror (" receiver : mkfifo " );
 exit (6) ;
 }
 }

 //dhmiourgia tou pipe out
 if ( mkfifo (fifo_out, 0666) == -1 ) {
 if ( errno != EEXIST ) {
 perror ( " receiver : mkfifo " ) ;
 exit (6) ;
 }
 }

 //o coordd anoigei to in gia diavasma
 // den anoigei to out gia grapsimo gt ama den exei anoixtei gia diavasma apo to console tha kollhsei
 if ( ( fd1 = open ( fifo_in, O_RDONLY | O_NONBLOCK ) ) < 0) {
 perror ( " fifo open problem " ) ;
 exit (3) ;
 }


 int flag = 0; 			//xrhsimopoieitai gia na ginei to anoigma gia grapsimo mono mia fora
 
 pid_t poolpid;
 int pool_number = 0;		// o arithmos twn pools pou exoun dhmiourghthei
 int last_pool_jobs = 0;	// posa jobs exoun dothei sto teleytaio pool
 char pool_fifo_in[10];
 char pool_fifo_out[10];
 int logical_job_number= 0;

 strcpy(pool_fifo_in,"fifo_in");
 strcpy(pool_fifo_out,"fifo_out");

 char fifo_buffer[10];

 								// o pinakas opou kratountai ta file descriptors twn pipes 
 pool_info** Array = new pool_info*[MAX_POOLS];		
 for(int i=0; i<MAX_POOLS; i++)
 Array[i] = new pool_info(job_number);

 int fd_in;
 int fd_out; 

 for (;;) 
 	{

 		if ( (read_bytes = read ( fd1 , msgbuf , MSGSIZE)) == 0) 
		{ 
		 continue;
		}
 		else if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
 		else if (read_bytes == -1)							//prwth fora pou o coord diavazei  
		{ 

		for(int j = 0; j<pool_number; j++)		         // oso lamvanw -1 koitaw ama kapoio pool thelei
				{					//na kanei exit
				
				read_bytes = read ( Array[j]->fd_in , msgbuf , MSGSIZE);
		 		if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
				if (read_bytes > 0) // otan vrei ena pool pou thelei na kanei exit
				{

					char tmp[MSGSIZE];
					strcpy(tmp,msgbuf);

					char* ex = strtok(tmp," ");

					if (strcmp(ex,"ex")==0) // kapoio pool thelei na termatisei
					{
						Array[j]->status = 0;
						strcpy(msgbuf,"exit");

						cout<<"(Coord exit response for pool): "<<Array[j]->pid<<endl;

						for(int k=0; k<job_number; k++)
						{
							Array[j]->exit_times[k] = atoi(strtok(NULL," "));
						}

						if (( nwrite = write ( Array[j]->fd_out , msgbuf , MSGSIZE) ) == -1)
 					 	{ perror ( " Error in Writing " ) ; exit (2) ; }
						//tou stelnei thn adeia
						

					}
				
					
				}
				
				}
		if (flag==0) 
                { 
		flag = 1; 
		cout<<"first special -1 read"<<endl;
 		}

		}

		if (flag == 1) 
			{
 			if ( ( fd2 = open ( fifo_out, O_WRONLY) ) < 0) {		        //o coord anoigei to out gia grapsimo
			perror ( " fifo open error " ) ;					//thn prwth fora pou diavazei epityxws
 			exit (1);
 			}
 			flag =2;
 			} 


 		if (read_bytes > 0) 					

		{
			cout<<"--> Coord received: "<<msgbuf<<endl;
 		 
			char msg[80];
			strcpy(msg,msgbuf);

			char* command = strtok(msg," ");

		if (strcmp(command,"submit") == 0)

			{

			logical_job_number++;

			/////////Se afto to shmeio o coord prepei na kanei submit sto katallhlo pool to job pou elave ///////////////

			if (last_pool_jobs < job_number && pool_number!=0  ) // to teleytaio pool mporei na e3yphrethsei akoma kapoia douleia
			{
				last_pool_jobs++;
				cout<<"Stelnei se hdh yparxon pool"<<endl;
				
				// stelnei thn entolh sto pipe tou teleytaiou pool
				// aftou dhl pou mporei na dextei mia akoma entolh

				if (( nwrite = write ( Array[pool_number - 1]->fd_out ,msgbuf, MSGSIZE ) ) == -1)
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
			}
			else // prepei na ginei fork() dhmiourgwntas neo pool
			{
				//dhmiourgia fifo gia to neo pool
				snprintf(fifo_buffer, sizeof(fifo_buffer), "%s%d",pool_fifo_in,pool_number);

				size_t len1 = strlen(path);
 				size_t len2 = strlen(fifo_buffer);

 				char totalPath1[100];

				memcpy(totalPath1,        path, len1);
				memcpy(totalPath1 + len1, fifo_buffer, len2);
				totalPath1[len1 + len2] = '\0';

				if (mkfifo(totalPath1,0666) == -1 ) {
 				if ( errno != EEXIST ) {
 				perror (" receiver : mkfifo " );
 				exit (6) ;
 				}
 				}

				snprintf(fifo_buffer, sizeof(fifo_buffer), "%s%d",pool_fifo_out,pool_number);

				len1 = strlen(path);
 				len2 = strlen(fifo_buffer);

 				char totalPath2[100];

				memcpy(totalPath2,        path, len1);
				memcpy(totalPath2 + len1, fifo_buffer, len2);
				totalPath2[len1 + len2] = '\0';

				if (mkfifo(totalPath2,0666) == -1 ) {
 				if ( errno != EEXIST ) {
 				perror (" receiver : mkfifo " );
 				exit (6) ;
 				}
 				}
				
				if ( ( fd_in = open (totalPath1 , O_RDONLY | O_NONBLOCK) ) < 0) {		        
				perror ( " fifo open error " ) ;	
 				exit (1);
 				}
				
				// katagrafh twn fds ston pinaka

				Array[pool_number]->fd_in = fd_in;
				
				if ( ( poolpid=fork() ) == -1) {
				perror ( " Failed to fork " ) ;
				exit (1) ;
				}
				else if ( poolpid == 0)
				{
					char num[3];
					char num2[6];
					sprintf (num2, "%d", logical_job_number);	
					sprintf (num, "%d", job_number);
					cout<<"~>Dhmiourgia pool"<<endl;
					execlp("./pool","pool",totalPath1,totalPath2,num,path,num2,(char *)NULL);
				}
				else
				{
				if ( ( fd_out = open (totalPath2 , O_WRONLY) ) < 0) {		        
				perror ( " fifo open error " ) ;	
 				exit (1);
 				}


				if (( nwrite = write ( fd_out ,msgbuf, MSGSIZE ) ) == -1) // stelnei thn entolh sto pipe tou pool
 				{ perror ( " Error in Writing " ) ; exit (2) ; }

				Array[pool_number]->fd_out = fd_out;
				Array[pool_number]->pid = poolpid;

				pool_number++;
				last_pool_jobs= 1;
				}


			}
		
				// ocoord koitazei gia apanthseis apo ta pools
				int j=0;
				for(;;)
				{
				
				read_bytes = read ( Array[j]->fd_in , msgbuf , MSGSIZE);
		 		if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
				if (read_bytes > 0) // otan vrei mia apanthsh kanei break
				{

					char tmp[MSGSIZE];
					strcpy(tmp,msgbuf);

					char* ex = strtok(tmp," ");

					if (strcmp(ex,"ex")==0) // kapoio pool thelei na termatisei
					{
						Array[j]->status = 0;
						strcpy(msgbuf,"exit");

						cout<<"(Coord exit response for pool): "<<Array[j]->pid<<endl;

						for(int k=0; k<job_number; k++)
						{
							Array[j]->exit_times[k] = atoi(strtok(NULL," "));
						}

						if (( nwrite = write ( Array[j]->fd_out , msgbuf , MSGSIZE) ) == -1)
 					 	{ perror ( " Error in Writing " ) ; exit (2) ; }
						//tou stelnei thn adeia
						

					}
					else
					{

					if (( nwrite = write ( fd2 , msgbuf , MSGSIZE) ) == -1) //kai th stelnei ston console
 					{ perror ( " Error in Writing " ) ; exit (2) ; }

					break;

					}
					
				}
				j++;
				if (j==pool_number) j=0;
				
				}
	

			 	
 			
		}
		else if (strcmp(command,"status") == 0) 
				{
					int job_no = atoi(strtok(NULL," "));

					
				if (job_no <= logical_job_number)
				{
					int position;
					if (job_no%job_number == 0) position = job_no/job_number - 1;
					else position = job_no/job_number;

					cout<<"Chosen position: "<<position<<endl;


					if (Array[position]->status == 0 ) 
					{
						cout<<"Pool with pid: "<<Array[position]->pid<<" is dead cannot respond"<<endl;	
						sprintf(msgbuf,"JobID: %d Status: Finished",job_no);
						if (( nwrite = write ( fd2 , msgbuf , MSGSIZE) ) == -1)  //request ston console
 						{ perror ( " Error in Writing " ) ; exit (2) ; }

					
					}
					else
					{
						if (( nwrite = write ( Array[position]->fd_out ,msgbuf, MSGSIZE ) ) == -1) // stelnei to erwthma
 				                { perror ( " Error in Writing " ) ; exit (2) ; }			   //sto katallhlo pool


						// elegxei gia apanthsh apo to pool pou ekane to erwthma
						for(;;)
						{
				
						read_bytes = read ( Array[position]->fd_in , msgbuf , MSGSIZE);
		 				if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
						if (read_bytes > 0 ) // otan vrei mia apanthsh kanei break
						{
							if (( nwrite = write ( fd2 , msgbuf , MSGSIZE) ) == -1) //kai th stelnei ston console
 							{ perror ( " Error in Writing " ) ; exit (2) ; }

							break;
					
						}
					
						}			 	

					}
				}
				else
				{
					if (( nwrite = write ( fd2 , "Invalid job number!!" , MSGSIZE) ) == -1)
 					{ perror ( " Error in Writing " ) ; exit (2) ; }

					
				}
			
		}
		else if (strcmp(command,"suspend") == 0) 
				{
				int job_no = atoi(strtok(NULL," "));

					
				if (job_no <= logical_job_number)
				{
					int position;
					if (job_no%job_number == 0) position = job_no/job_number - 1;
					else position = job_no/job_number;


					if (Array[position]->status == 0 ) 
					{
						cout<<"Pool with pid: "<<Array[position]->pid<<" is dead cannot respond"<<endl;	
						sprintf(msgbuf,"JobID: %d Status: Finished",job_no);
						if (( nwrite = write ( fd2 , msgbuf , MSGSIZE) ) == -1)  //request ston console
 						{ perror ( " Error in Writing " ) ; exit (2) ; }

					
					}
					else
					{
						if (( nwrite = write ( Array[position]->fd_out ,msgbuf, MSGSIZE ) ) == -1) // stelnei to erwthma
 				                { perror ( " Error in Writing " ) ; exit (2) ; }			   //sto katallhlo pool


						// elegxei gia apanthsh apo to pool pou ekane to erwthma
						for(;;)
						{
				
						read_bytes = read ( Array[position]->fd_in , msgbuf , MSGSIZE);
		 				if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
						if (read_bytes > 0 ) // otan vrei mia apanthsh kanei break
						{
							if (( nwrite = write ( fd2 , msgbuf , MSGSIZE) ) == -1) //kai th stelnei ston console
 							{ perror ( " Error in Writing " ) ; exit (2) ; }

							break;
					
						}
					
						}			 	

					}
				}
				else
				{
					if (( nwrite = write ( fd2 , "Invalid job number!!" , MSGSIZE) ) == -1)
 					{ perror ( " Error in Writing " ) ; exit (2) ; }

					
				}
			
		}
		else if (strcmp(command,"resume") == 0) 
				{
				int job_no = atoi(strtok(NULL," "));

					
				if (job_no <= logical_job_number)
				{
					int position;
					if (job_no%job_number == 0) position = job_no/job_number - 1;
					else position = job_no/job_number;


					if (Array[position]->status == 0 ) 
					{
						cout<<"Pool with pid: "<<Array[position]->pid<<" is dead cannot respond"<<endl;	
						sprintf(msgbuf,"JobID: %d Status: Finished",job_no);
						if (( nwrite = write ( fd2 , msgbuf , MSGSIZE) ) == -1)  //request ston console
 						{ perror ( " Error in Writing " ) ; exit (2) ; }

					
					}
					else
					{
						if (( nwrite = write ( Array[position]->fd_out ,msgbuf, MSGSIZE ) ) == -1) // stelnei to erwthma
 				                { perror ( " Error in Writing " ) ; exit (2) ; }			   //sto katallhlo pool


						// elegxei gia apanthsh apo to pool pou ekane to erwthma
						for(;;)
						{
				
						read_bytes = read ( Array[position]->fd_in , msgbuf , MSGSIZE);
		 				if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
						if (read_bytes > 0 ) // otan vrei mia apanthsh kanei break
						{
							if (( nwrite = write ( fd2 , msgbuf , MSGSIZE) ) == -1) //kai th stelnei ston console
 							{ perror ( " Error in Writing " ) ; exit (2) ; }

							break;
					
						}
					
						}			 	

					}
				}
				else
				{
					if (( nwrite = write ( fd2 , "Invalid job number!!" , MSGSIZE) ) == -1)
 					{ perror ( " Error in Writing " ) ; exit (2) ; }

					
				}
			
		}
		else if (strcmp(command,"status-all") == 0) 
			{
				char* dur = strtok(NULL," ");

				char final_buf[MSGSIZE];
				int pool_flag = 0;
				int length = 0;
				
				if (dur==NULL)
				{
					for(int j=0; j<pool_number; j++)
					{
					if (Array[j]->status == 1)
					{
						pool_flag++;
						if (( nwrite = write ( Array[j]->fd_out ,"status-all", MSGSIZE ) ) == -1) 
 				                { perror ( " Error in Writing " ) ; exit (2) ; }			 //sto katallhlo pool
					}
					else
					{
						for(int fin_jobs = 0; fin_jobs<job_number; fin_jobs++)
						{
						sprintf(msgbuf,"%d Finished\n",j*job_number + fin_jobs + 1);
						length += sprintf(final_buf+length,"%s", msgbuf);
						}
						cout<<endl<<"from coord"<<final_buf<<endl;
				
					}				
					}

					sleep(1);

					int pos=0;
					while(pool_flag)
					{
					read_bytes = read ( Array[pos]->fd_in , msgbuf , MSGSIZE);
		 			if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
					if (read_bytes > 0 ) // otan vrei mia apanthsh kanei break
					{

					length += sprintf(final_buf+length,"%s",msgbuf);
		
					pool_flag--;
					
					}

					pos++;
					if (pos==pool_number) pos=0;

					}

					
			
				}//time == NULL end
			 	else
				{
					int duration = atoi(dur);
					cout<<"dur: "<<dur<<endl;

					char command[MSGSIZE];
					sprintf(command,"%s %d","status-all",duration);

					cout<<"commsnd: "<<command<<endl;
					

					for(int j=0; j<pool_number; j++)
					{
					if (Array[j]->status == 1)
					{
						pool_flag++;
						if (( nwrite = write ( Array[j]->fd_out ,command, MSGSIZE ) ) == -1) 
 				                { perror ( " Error in Writing " ) ; exit (2) ; }			 //sto katallhlo pool
					}
					else
					{
						for(int fin_jobs = 0; fin_jobs<job_number; fin_jobs++)
						{

						if ( ( time(NULL) - Array[j]->exit_times[fin_jobs] ) <= duration)
						{
							sprintf(msgbuf,"%d Finished\n",j*job_number + fin_jobs + 1);
							length += sprintf(final_buf+length,"%s", msgbuf);
						}

						}
						cout<<endl<<"from coord"<<final_buf<<endl;
				
					}				
					}

					sleep(1);

					int pos=0;
					while(pool_flag)
					{
					read_bytes = read ( Array[pos]->fd_in , msgbuf , MSGSIZE);
		 			if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
					if (read_bytes > 0 ) // otan vrei mia apanthsh kanei break
					{

					length += sprintf(final_buf+length,"%s",msgbuf);
		
					pool_flag--;
					
					}

					pos++;
					if (pos==pool_number) pos=0;

					}

					

					
					
				}

				if (( nwrite = write ( fd2 , final_buf , MSGSIZE) ) == -1) //kai th stelnei ston console
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
					



			}
			else if (strcmp(command,"show-pools") == 0) 
			{

				char final_buf[MSGSIZE];
				int pool_flag = 0;
				int length = 0;
					cout<<"Pool num: "<<pool_number<<endl;
					for(int j=0; j<pool_number; j++)
					{
					if (Array[j]->status == 1)
					{
						pool_flag++;
						if (( nwrite = write ( Array[j]->fd_out ,"show-pools", MSGSIZE ) ) == -1) 
 				                { perror ( " Error in Writing " ) ; exit (2) ; }			 //sto katallhlo pool
					}	
					}

					sleep(2);

					if (pool_flag==0) sprintf(final_buf,"%s","No pools active");

					int pos=0;
					while(pool_flag)
					{
					read_bytes = read ( Array[pos]->fd_in , msgbuf , MSGSIZE);
		 			if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
					if (read_bytes > 0  ) // otan vrei mia apanthsh kanei break
					{

					length += sprintf(final_buf+length,"%s",msgbuf);
		
					pool_flag--;
					
					}

					pos++;
					if (pos==pool_number) pos=0;

					}

				cout<<"Coord final buf: "<<final_buf<<endl;
					

				if (( nwrite = write ( fd2 , final_buf , MSGSIZE) ) == -1) //kai th stelnei ston console
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
					

			}
			else if (strcmp(command,"show-active") == 0) 
			{
				char final_buf[MSGSIZE];
				int pool_flag = 0;
				int length = 0;
				
					for(int j=0; j<pool_number; j++)
					{
					if (Array[j]->status == 1)
					{
						pool_flag++;
						if (( nwrite = write ( Array[j]->fd_out ,"show-active", MSGSIZE ) ) == -1) 
 				                { perror ( " Error in Writing " ) ; exit (2) ; }			 //sto katallhlo pool
					}		
					}

					if (pool_flag==0) sprintf(final_buf,"%s","No active jobs");

					int pos=0;
					while(pool_flag)
					{
					read_bytes = read ( Array[pos]->fd_in , msgbuf , MSGSIZE);
		 			if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
					if (read_bytes > 0 ) // otan vrei mia apanthsh kanei break
					{

					length += sprintf(final_buf+length,"%s",msgbuf);
		
					pool_flag--;
					
					}

					pos++;
					if (pos==pool_number) pos=0;

					}

					cout<<"Coord final buf: "<<final_buf<<endl;

					
				if (( nwrite = write ( fd2 , final_buf , MSGSIZE) ) == -1) //kai th stelnei ston console
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
					

			}
		else if (strcmp(command,"show-finished") == 0) 
			{

				char final_buf[MSGSIZE];
				int pool_flag = 0;
				int length = 0;

					sprintf(final_buf,"%s\n","Finished:");
				
					for(int j=0; j<pool_number; j++)
					{

					if (Array[j]->status == 1)
					{
						pool_flag++;
						if (( nwrite = write ( Array[j]->fd_out ,"show-finished", MSGSIZE ) ) == -1) 
 				                { perror ( " Error in Writing " ) ; exit (2) ; }			 //sto katallhlo pool
					}
					else
					{
						for(int fin_jobs = 0; fin_jobs<job_number; fin_jobs++)
						{
						sprintf(msgbuf,"%d Finished\n",j*job_number + fin_jobs + 1);
						length += sprintf(final_buf+length,"%s", msgbuf);
						}
						cout<<"from coord"<<final_buf<<endl;
				
					}				
					}

					int pos=0;
					while(pool_flag)
					{
					read_bytes = read ( Array[pos]->fd_in , msgbuf , MSGSIZE);
		 			if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
					if (read_bytes > 0 ) // otan vrei mia apanthsh kanei break
					{

					length += sprintf(final_buf+length,"%s",msgbuf);
		
					pool_flag--;
					
					}
					

					pos++;
					if (pos==pool_number) pos=0;

					}

					cout<<"Coord final buf: "<<final_buf<<endl;


				if (( nwrite = write ( fd2 , final_buf , MSGSIZE) ) == -1) //kai th stelnei ston console
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
					



			}
			else if (strcmp(command,"shutdown") == 0) 
			{

				int pool_flag = 0;

				for(int j=0; j<pool_number; j++)
					{

					if (Array[j]->status == 1)
					{
						pool_flag++;
						kill(Array[j]->pid,SIGTERM);
					}
					}

				int pos=0;
				int jobs_in_progress = 0;

					while(pool_flag)
					{
					read_bytes = read ( Array[pos]->fd_in , msgbuf , MSGSIZE);
		 			if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
					if (read_bytes > 0 )
					{
		
					jobs_in_progress+=atoi(msgbuf);

					pool_flag--;
										
					}
					

					pos++;
					if (pos==pool_number) pos=0;

					}

					if (( nwrite = write ( fd2 , "coord-exit" , MSGSIZE) ) == -1) //kai th stelnei ston console
	 				{ perror ( " Error in Writing " ) ; exit (2) ; }

					cout<<"Coord exited: served "<<logical_job_number<<" "<<jobs_in_progress<<" were still in progress"<<endl;
					for(int fin = 0; fin<MAX_POOLS; fin++)
					delete Array[fin];
					delete [] Array;

					delete path;
					delete fifo_out;
					delete fifo_in;
		
					exit(1);
				
				
			}
			else
			{
				if (( nwrite = write ( fd2 , "Invalid Command!" , MSGSIZE) ) == -1) //kai th stelnei ston console
 				{ perror ( " Error in Writing " ) ; exit (2) ; }	
			}

				

		}// read > 0 end

		int status;
		int exited_pool;
		if( ( exited_pool = waitpid(0,&status,WNOHANG) ) !=0 )
		{ 
		if (exited_pool!=-1)
		{
			cout<<"(Coord check) Pool: "<<exited_pool<<" exited"<<endl;
			
		}
		}

		
 
		}// for end

 
 

}// coord end

