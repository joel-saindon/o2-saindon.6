#include "info.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

int main (int argc, char * argv[]){
	bool optflag = true;
	bool logFlag = true;
	char * defval = "5";
	char * defLog = "logfile";
	char * temp = NULL;

	int opt;
	while((opt = getopt(argc,argv, "hs:l:t")) != -1){
		switch (opt){
			case 'h':
				printf("Help Text");
				return -1;
			case 's':
				printf("s option, args %d \n", argc);
				//printf("optarg: %s\n", optarg);
				int test = atoi(argv[2]);
				maxproc = test;
				optflag = false;
				break;
			case 'l':
				printf("l option\n");
				printf("%s\n", optarg);
				//temp = optarg;
				//logfile = optarg;
				char * mainlogfile = argv[4];
				printf("%s\n", logfile);
				logFlag = false;
				break;
			case 't'://testing only
				break;
			case '?':
				printf("Usage: \n");
				return -1;
			default:
				return -1;	
		}
	}

	FILE *logFromMain;
	//if no logfile given, open default logfile
	if(logFlag == true){
		logFromMain = fopen(defLog, "w");
	} else { // if logfile IS given, open it
	logFromMain = fopen(argv[4], "w");
	}
	
	//check if given maxproc is over 18
	if(maxproc > 18){
		optflag = true;
		fprintf(logFromMain, "Given maxproc over 18, resorting to defaults..\n");
	}

	fprintf(logFromMain, "Preparing to run OSS\n");
	fclose(logFromMain);
	//printf("maxproc: %d\n", maxproc);
	int status;
	pid_t childpid = 0;
	pid_t waitreturn;
	int i = 0;
	/*maxproc always passed to oss as 1st real argument (excludes ./oss), logfile always passed as second*/
	for(i=0; i< 1; i++){
		if((childpid = fork()) < 0){
			perror("fork");
			fprintf(logFromMain, "Fork Error, Terminating..");
			return -1;
		}
		//if both a logfile and a max proc is given
		else if(optflag == false && logFlag == false && childpid == 0){
			fprintf(logFromMain, "child created: oss-> %d\n", getpid());
			fprintf(logFromMain, "running with given logfile %s and given maxproc %d\n", argv[4], argv[2]);
			//fclose(logFromMain);
			execl("./oss", "./oss", (char*)argv[2], (char*) argv[4], (char*)NULL);
			exit(0);
		}
		//if maxproc is given but logfile is not
		else if(optflag == false && logFlag == true &&  childpid == 0){
			fprintf(logFromMain, "child created: oss-> %d\n", getpid());
			fprintf(logFromMain, "running with given maxproc %d and default logfile\n", argv[2]);
			//fclose(logFromMain);
			execl("./oss", "./oss", (char *) argv[2], defLog, (char*) NULL);
			exit(0);
		}
		//if logfile is given but maxproc is not
		else if (optflag == true && logFlag == false && childpid == 0){
			fprintf(logFromMain, "child created: oss-> %d\n", getpid());
			fprintf(logFromMain, "running with given logfile %s and default maxproc\n", argv[2]);
			//fclose(logFromMain);
			execl("./oss", "./oss", defval, (char *) argv[2], (char*) NULL);
			exit(0);
		}	
		//if neither maxproc or logfile is given
		else if(optflag == true && logFlag == true &&  childpid == 0){
			fprintf(logFromMain, "child created: oss-> %d\n", getpid());
			fprintf(logFromMain, "running with no maxproc or logfile set, using defaults\n");
			//fclose(logFromMain);
			execl("./oss", "./oss", defval, defLog, (char*) NULL);
			exit(0);
		}
		else {
			(void)waitpid(childpid, &status, 0);
			//fprintf(logFromMain, "Main terminating..\n");
			//fclose(logFromMain);
		}
	}

	//fclose(logFromMain);
return 0;
}

