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


using namespace std;

int main(int argc, char* argv[]) {


if (argc!=2) { cerr<<"Error in arguments!"<<endl; return 0; }

int sleep_time = atoi(argv[1]);

cout<<"I am sleeping for "<<sleep_time<<" secs!!"<<endl;

sleep(sleep_time);

return 0;
}
