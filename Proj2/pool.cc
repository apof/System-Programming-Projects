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


using namespace std;

static int* global_pid;
static int* global_status;
static int global_fd;
static int global_max_jobs;

struct job_info {

	int pid;
	long int start_time;
	int status;
	long int time_suspended;
	long int suspend_start;


	job_info() 
	{ 
		pid = -1; 
		start_time = -1; 
		status = 1;
		suspend_start = -1;
		time_suspended = 0; 
	}
};


void sig_handler(int signo)
{
  if (signo == SIGTERM) {

  int sig = 0;
  int still_in_prog = 0;

	for(int sig=0; sig<global_max_jobs; sig++)
	{
		if (global_status[sig]==1) {
		kill(global_pid[sig],SIGTERM);
		still_in_prog++;
		cout<<"Job with pid: "<<global_pid[sig]<<" exited with SIGTERM"<<endl;
		}
	}
	char prog_num[4];
	sprintf(prog_num,"%d",still_in_prog);

	if (write ( global_fd ,prog_num, MSGSIZE )  == -1)
	// stelnei ena response pisw ston coord
 	{ perror ( " Error in Writing " ) ; exit (2) ; }

	cout<<"Pool with id: "<<getpid()<<" exited (SIGTERM Exit)"<<endl;

	delete global_pid;
	delete global_status;

	exit(1); 
      }
}



int main(int argc, char *argv[]) {

 int job_counter = 0;
 int job_submit = 0;

 char msgbuf[MSGSIZE];
 char request_buff[MSGSIZE];


 char* fifo_in = new char[strlen(argv[2]) + 1];
 char* fifo_out = new char[strlen(argv[1]) + 1];
 int max_jobs = atoi(argv[3]);
 char* path = new char[strlen(argv[4]) + 1];
 int logical_job_number = atoi(argv[5]);

 int start_logical_job_number = logical_job_number;

 global_max_jobs = max_jobs;

 global_pid = new int[max_jobs];
 global_status = new int[max_jobs];

 for(int g=0; g<max_jobs; g++) global_status[g] = -1;

 strcpy(fifo_in,argv[2]);
 strcpy(fifo_out,argv[1]);
 strcpy(path,argv[4]);

 job_info** job_Array = new job_info*[max_jobs];
 for (int g=0; g<max_jobs; g++) job_Array[g] = new job_info;

 int mypid;
 int fd1,fd2;
 int read_bytes, nwrite;

 char file[20];
 strcpy(file,"sdi1400217_");

 if ( ( fd1 = open ( fifo_in, O_RDONLY | O_NONBLOCK ) ) < 0) {
 perror ( " fifo open problem " ) ;
 exit (3) ;
 }

 if ( ( fd2 = open ( fifo_out, O_WRONLY) ) < 0) {
 perror ( " fifo open problem " ) ;
 exit (3) ;
 }

 global_fd = fd2;

 cout<<"~~~>Id of pool: "<<getpid()<<" In pipe: "<<fifo_in<<" Out pipe: "<<fifo_out<< " fd in out " <<fd1<<fd2<<endl;

 if (signal(SIGTERM, sig_handler) == SIG_ERR)
  printf("\ncan't catch SIGTERM\n");

 for (;;) 
 	{

 		if ( (read_bytes = read ( fd1 , msgbuf , MSGSIZE)) == 0) { continue; }
 		else if (read_bytes < 0 && read_bytes != -1 ) perror ( " Coord read problem " ) ;
		else if (read_bytes > 0)
		{

				
			cout<<"-->Pool with id: "<<getpid()<<" read: "<<msgbuf<<endl;
			char* comm = strtok(msgbuf," ");

			if (strcmp(comm,"exit")==0)
			{
				cout<<"(Pool exit) Pool: "<<getpid()<<" exited!"<<endl; 

				delete global_pid;
				delete global_status;
				delete path;
				delete fifo_in;
				delete fifo_out;
				for(int fin=0; fin<max_jobs; fin++) delete job_Array[fin];
				delete [] job_Array;

				return 0;
			}
			else if (strcmp(comm,"submit")==0)
			{
			
			int child_id;
			
			if ((child_id = fork())==0) 
			{
				//creating exec command buffer
	
				char* pch;
				char* command;
				char** exec_buff = new char*[10];
	
				int i = 0;
				command = strtok (NULL," ");
				exec_buff[i] = strdup(command);
				i++;
	
				while (pch != NULL)
				{
				pch = strtok (NULL, " ");
				if (pch!=NULL) exec_buff[i] = strdup(pch);
				i++;
				}

				exec_buff[i] = NULL;

				char file_buffer[80];
				char file[20];
				strcpy(file,"sdi1400217_");

				snprintf(file_buffer, sizeof(file_buffer), "%s%d",file,logical_job_number);

				int k=0;
				while(file_buffer[k]!='\0') k++;
				file_buffer[k] = '_';
				file_buffer[k+1] = '\0';

				char tmp[80];
				strcpy(tmp,file_buffer);
				
				snprintf(file_buffer, sizeof(file_buffer), "%s%d",tmp,getpid());
				

				k=0;
				while(file_buffer[k]!='\0') k++;
				file_buffer[k] = '_';
				file_buffer[k+1] = '\0';

			 	time_t t = time(NULL);
				struct tm tm = *localtime(&t);

				char year[5];
				char month[3];
				char day[2];
				char hour[3];
				char min[3];
				char sec[3];

				sprintf(year,"%d",tm.tm_year + 1900);
				sprintf(month,"%d",tm.tm_mon + 1);
				sprintf(day,"%d",tm.tm_mday);
				sprintf(hour,"%d",tm.tm_hour);
				sprintf(min,"%d",tm.tm_min);
				sprintf(sec,"%d",tm.tm_sec);

				size_t len1 = strlen(year);
 				size_t len2 = strlen(month);

 				char total1[20];

				memcpy(total1,        year, len1);
				memcpy(total1 + len1, month, len2);
				total1[len1 + len2] = '\0';

				len1 = strlen(total1);
 				len2 = strlen(day);

 				char total2[20];

				memcpy(total2,        total1, len1);
				memcpy(total2 + len1, day, len2);
				total2[len1 + len2] = '\0';

				len1 = strlen(file_buffer);
 				len2 = strlen(total2);

 				char total3[20];

				memcpy(total3,        file_buffer, len1);
				memcpy(total3 + len1, total2, len2);
				total3[len1 + len2] = '\0';

				k=0;
				while(total3[k]!='\0') k++;
				total3[k] = '_';
				total3[k+1] = '\0';


				len1 = strlen(hour);
 				len2 = strlen(min);


				memcpy(total1,        hour, len1);
				memcpy(total1 + len1, min, len2);
				total1[len1 + len2] = '\0';

				len1 = strlen(total1);
 				len2 = strlen(sec);


				memcpy(total2,        total1, len1);
				memcpy(total2 + len1, sec, len2);
				total2[len1 + len2] = '\0';

				len1 = strlen(total3);
 				len2 = strlen(total2);

 				char final_file[30];

				memcpy(final_file,        total3, len1);
				memcpy(final_file + len1, total2, len2);
				final_file[len1 + len2] = '\0';

				len1 = strlen(path);
 				len2 = strlen(final_file);

 				char final_path[100];

				memcpy(final_path,        path, len1);
				memcpy(final_path + len1, final_file, len2);
				final_path[len1 + len2] = '\0';

				cout<<"Job with logic id: "<<logical_job_number<<" and PID: "<<getpid()<< " ready to execute"<<endl;

				struct stat st = {0}; //dhmiourgia tou directory gia to output


 				if (stat(final_file, &st) == -1) {
    				mkdir(final_path, 0700);
 				}

				char file_out[20];
				char file_err[20];

				sprintf(file_out,"stdout_%d",getpid());
				sprintf(file_err,"stderr_%d",getpid());

				char final_file_out[100];
				char final_file_err[100];

				sprintf(final_file_out,"%s/%s",final_path,file_out);
				sprintf(final_file_err,"%s/%s",final_path,file_err);

				
				int result_fd_out = creat(final_file_out,O_RDWR | O_CREAT | S_IRWXU|S_IWUSR|S_IRGRP|S_IROTH);  
				// creating the output file
				int result_fd_err = creat(final_file_err,O_RDWR | O_CREAT | S_IRWXU|S_IWUSR|S_IRGRP|S_IROTH);  
				// creating the error file


				dup2(result_fd_out,1);					     // redirecting stderr and stdout sta
				dup2(result_fd_err,2);					     //katallhla files
				close(result_fd_out);
				close(result_fd_err);

				memset(request_buff, 0, sizeof(request_buff));
				

				sprintf(request_buff,"JobID: %d PID: %d",logical_job_number,getpid()); 

				if (( nwrite = write ( fd2 ,request_buff, MSGSIZE ) ) == -1)
				// stelnei ena response pisw ston coord
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
				
				if ( execvp(command,exec_buff) < 0 )
				{
					char execv_buffer[50];
					sprintf(execv_buffer,"./%s",command);
					execvp(execv_buffer,exec_buff);
				}

				
				 
			}
			else 
			{
			logical_job_number++; 
			job_Array[job_submit]->pid = child_id;
			global_pid[job_submit] = child_id;
			global_status[job_submit] = 1;
			job_Array[job_submit]->start_time = time(NULL); 
			job_submit++;
			}
			}
			else if (strcmp(comm,"status")==0)
			{

				memset(request_buff, 0, sizeof(request_buff));

				int job_no = atoi(strtok (NULL," "));
				int position = job_no - start_logical_job_number;

				cout<<job_Array[position]->status<<" "<<job_Array[position]->pid<<endl;

				if (job_Array[position]->status == 1)
				sprintf(request_buff,"JobID: %d Status: Active ( running for %ld secs )",job_no,time(NULL) - job_Array[position]->start_time - job_Array[position]->time_suspended);

				else if (job_Array[position]->status == 0)
				sprintf(request_buff,"JobID: %d Status: Finished",job_no);
				else if (job_Array[position]->status == 2)
				sprintf(request_buff,"JobID: %d Status: Suspended",job_no);
				if (( nwrite = write ( fd2 ,request_buff, MSGSIZE)  ) == -1)
				// stelnei ena response pisw ston coord
 				{ perror ( " Error in Writing " ) ; exit (2) ; }				
				
					
			}
			else if (strcmp(comm,"suspend")==0)
			{
				int job_no = atoi(strtok (NULL," "));
				int position = job_no - start_logical_job_number;

				memset(request_buff, 0, sizeof(request_buff));

				if ( job_Array[position]->status == 1)
				{

				job_Array[position]->status = 2;

				job_Array[position]->suspend_start = time(NULL);

				kill(job_Array[position]->pid,SIGSTOP);


				sprintf(request_buff,"Sent suspend signal to JobID %d",job_no);

				}
				else
				{
					if (job_Array[position]->status == 0)
					sprintf(request_buff,"Cannot suspend: Job with JobID %d is finished",job_no);
					else if (job_Array[position]->status == 2)
					sprintf(request_buff,"Cannot suspend: Job with JobID %d is already suspended",job_no);

				}

				if (( nwrite = write ( fd2 ,request_buff, MSGSIZE)  ) == -1)
				// stelnei ena response pisw ston coord
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
				

								
			}
			else if (strcmp(comm,"resume")==0)
			{
				int job_no = atoi(strtok (NULL," "));
				int position = job_no - start_logical_job_number;

				memset(request_buff, 0, sizeof(request_buff));

				if ( job_Array[position]->status == 2)
				{

				job_Array[position]->status = 1;

				job_Array[position]->time_suspended += ( time(NULL) - job_Array[position]->suspend_start );
				job_Array[position]->suspend_start = -1;

				kill(job_Array[position]->pid,SIGCONT);

				sprintf(request_buff,"Sent resume signal to JobID %d",job_no);

				}
				else
				{
					
				if (job_Array[position]->status == 0)
				sprintf(request_buff,"Cannot resume: Job with JobID %d is finished",job_no);
				else if (job_Array[position]->status == 1)
				sprintf(request_buff,"Cannot resume: Job with JobID %d is active",job_no);

				}

				if (( nwrite = write ( fd2 ,request_buff, MSGSIZE)  ) == -1)
				// stelnei ena response pisw ston coord
 				{ perror ( " Error in Writing " ) ; exit (2) ; }

				
			}
			else if (strcmp(comm,"status-all")==0)
			{
				char *dur = strtok (NULL," ");

				char final_buf[MSGSIZE];
				int length = 0;

				memset(request_buff, 0, sizeof(request_buff));
				memset(final_buf, 0, sizeof(final_buf));

				if (dur==NULL)
				{
				for(int position = 0; position<job_submit; position++)
				{
				if (job_Array[position]->status == 1)
				sprintf(request_buff,"%d Active (%ld secs)\n",start_logical_job_number + position,time(NULL) - job_Array[position]->start_time - job_Array[position]->time_suspended);

				else if (job_Array[position]->status == 0)
				sprintf(request_buff,"%d Finished\n",start_logical_job_number + position);
				else if (job_Array[position]->status == 2)
				sprintf(request_buff,"%d Suspended\n",start_logical_job_number + position);
				
				
				length += sprintf(final_buf+length,"%s",request_buff);

				}

				}
				else
				{
				int duration = atoi(dur);

				for(int position = 0; position<job_submit; position++)
				{
				if ((time(NULL) - job_Array[position]->start_time ) <= duration)
				{
				if (job_Array[position]->status == 1)
				sprintf(request_buff,"%d Active (%ld secs)\n",start_logical_job_number + position,time(NULL) - job_Array[position]->start_time - job_Array[position]->time_suspended);

				else if (job_Array[position]->status == 0)
				sprintf(request_buff,"%d Finished\n",start_logical_job_number + position);
				else if (job_Array[position]->status == 2)
				sprintf(request_buff,"%d Suspended\n",start_logical_job_number + position);
				
				
				length += sprintf(final_buf+length,"%s",request_buff);

				}
				}

					
				}

				cout<<"pool "<<getpid()<<" sent "<<endl<<final_buf<<endl;

				if (( nwrite = write ( fd2 ,final_buf, MSGSIZE)  ) == -1)
				// stelnei ena response pisw ston coord
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
					
				
			}
			else if (strcmp(comm,"show-pools")==0)
			{

				int length = 0;
				int active_jobs = 0;

				memset(request_buff, 0, sizeof(request_buff));

				for(int position = 0; position<job_submit; position++)
				{
				if (job_Array[position]->status == 1 || job_Array[position]->status == 2)
				active_jobs++;
				}

				sprintf(request_buff,"%d %d\n",getpid(),active_jobs);

				cout<<"pool "<<getpid()<<" sent "<<endl<<request_buff<<endl;

				if (( nwrite = write ( fd2 ,request_buff, MSGSIZE)  ) == -1)
				// stelnei ena response pisw ston coord
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
					
				
			}
			else if (strcmp(comm,"show-active")==0)
			{

				char final_buf[MSGSIZE];
				int length = 0;

				memset(request_buff, 0, sizeof(request_buff));
				memset(final_buf, 0, sizeof(final_buf));

				cout<<"Pool final buf at start: "<<final_buf<<endl;		
				cout<<"Pool req buf at start: "<<request_buff<<endl;		
				

				for(int position = 0; position<job_submit; position++)
				{
				if (job_Array[position]->status == 1)
				{ 
					sprintf(request_buff,"%d Active\n",start_logical_job_number + position);
					length += sprintf(final_buf+length,"%s",request_buff);
				}
				/*else if (job_Array[position]->status == 2)
				{ 
					sprintf(request_buff,"%d Suspended\n",start_logical_job_number + position); 
					length += sprintf(final_buf+length,"%s",request_buff); 
				}*/

				}
					
				cout<<"pool "<<getpid()<<" sent "<<endl<<final_buf<<endl;

				if (( nwrite = write ( fd2 ,final_buf, MSGSIZE)  ) == -1)
				// stelnei ena response pisw ston coord
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
					
				
			}
			else if (strcmp(comm,"show-finished")==0)
			{

				char final_buf[MSGSIZE];
				int length = 0;


				memset(request_buff, 0, sizeof(request_buff));
				memset(final_buf, 0, sizeof(final_buf));
				

				for(int position = 0; position<job_submit; position++)
				{

				if (job_Array[position]->status == 0)
				{
					sprintf(request_buff,"%d Finished\n",start_logical_job_number + position);
					length += sprintf(final_buf+length,"%s",request_buff);
				}

				}

				cout<<"pool "<<getpid()<<" sent "<<endl<<final_buf<<endl;

				if (( nwrite = write ( fd2 ,final_buf, MSGSIZE)  ) == -1)
				// stelnei ena response pisw ston coord
 				{ perror ( " Error in Writing " ) ; exit (2) ; }
				
			}


		}//read > 0 loop
	int status;
	int exited_job;
	if( ( exited_job = waitpid(0,&status,WNOHANG) ) !=0 )
	{ 
		if (exited_job != -1) {
		for(int l=0; l<max_jobs; l++)                             
			{
				if (job_Array[l]->pid == exited_job) {
				job_Array[l]->status = 0;
				global_status[l] = 0;
				}
			}
		cout<<"Job: "<<exited_job<<" exited"<<endl;
		job_counter++;
		if ( job_counter == max_jobs) 
		{  
			cout<<"(Pool exit request) Pool: "<<getpid()<< " reached max jobs and wants to exit!"<<endl;

			char exit_request[MSGSIZE];
			memset(exit_request, 0, sizeof(exit_request));

			char ex[4];
			sprintf(ex,"%s","ex"); 

			char tim[4];
			int length = 0;

			char times[MSGSIZE];

			for(int k=0; k<max_jobs; k++)
			{
				sprintf(tim,"%ld ",job_Array[k]->start_time);
				length += sprintf(times+length,"%s",tim);
			
			}

			sprintf(exit_request,"%s %s",ex,times);


			if (( nwrite = write ( fd2 ,exit_request, MSGSIZE)  ) == -1)
			// stelnei ena aithma gia exit pisw ston coord
 			{ perror ( " Error in Writing " ) ; exit (2) ; }
			 
		}
		}
	}
	
	}//for loop


}
