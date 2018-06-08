#include "info.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/msg.h>



int main (int argc, char * argv[]){
	signal(SIGINT, setdoneflag);
	alarm(10); //set manual timeout
	int i;
	int status;
	//FILE *logfile;
	logfile = fopen(argv[2], "a");
	fprintf(logfile, "OSS Started\n");
	//fclose(logfile);
	pid_t childpid =0;
	mymsg_t userMessage;
	userMessage.mtype =1;

	//define parameters for sig handler
	act.sa_handler = setdoneflag;
	act.sa_flags = 0;
	if(sigemptyset(&act.sa_mask) == -1 || (sigaction(SIGALRM, &act, NULL) == -1)){
		perror("SIGINT Handler failed");
	}

	//printf("from OSS\n");
	//printf("read from main.c: %d\n", atoi(argv[1]));
	maxproc = atoi(argv[1]);
	sleep(1);

	createShm();	
	
	/*
	*clock = 0;
	printf("clock: %d\n", *clock);
	*clock = *clock + 1;
	printf("clock: %d\n", *clock);
	int * test = clock;
	*test = *test + 1;
	*/

	*procs = maxproc;
	//printf("maxproc after attach %d\n", *procs);


	//fork user processes
	//printf("maxproc before fork loop: %d\n", maxproc);

	fprintf(logfile, "Forking children up to maximum %d\n", *procs);
	for(i=0; i< *procs; i++){
		if((childpid = fork()) < 0){
			perror("fork");
			return -1;
		}
		else if(childpid == 0){
			printf("child created %d\n", getpid());
			execl("./user", "./user", (char*)NULL);
			exit(0);
		}
		else {
			fprintf(logfile, "child created %d\n", childpid);
			OSSclk->nanoSec = OSSclk->nanoSec + 10000;
			fprintf(logfile, "logical clock incremented by 10000 nanosec: %d  %d\n", OSSclk->nanoSec, OSSclk->sec);
		}
	}
	//message loop
	int msgResult;	
	while(!doneflag){
		//sleep(1);
		if(*procs < maxproc){
			fprintf(logfile, "current procs(%d) less than maxproc (%d), forking...\n", *procs, maxproc);
			if((childpid= fork()) < 0){
				perror("fork");
				releaseMem();
				return -1; 
			}
			else if (childpid == 0){
				//printf("child created %d\n", getpid());
				execl("./user", "./user", (char*)NULL);
				exit(0);
			}
			else {
				fprintf(logfile, "child created %d\n", childpid);
				OSSclk->nanoSec = OSSclk->nanoSec + 10000;
				fprintf(logfile, "logical clock incremented by 10000 nanosec: %d  %d\n", OSSclk->nanoSec, OSSclk->sec);
			}
			
		}
		/*
			else{	
			fprintf(logfile, "current procs: %d", *procs);
		}
		*/
		printf("DEBUG: %d   %d   %d\n", *procs, maxproc, OSSclk->nanoSec);
		if((msgResult = msgrcv(shm_msgQueue, (void*)&userMessage , sizeof(mymsg_t), 0, MSG_NOERROR)) == -1){
			perror("msgrcv fail"); 
			break;
		}
		fprintf(logfile, "message received from child %d\n", userMessage.userPID);
		fprintf(logfile, "maxproc after message: %d\n",  userMessage.proc);
		fprintf(logfile, "Killing process %d\n", userMessage.userPID);
		kill(userMessage.userPID, SIGTERM);
		*procs = *procs -1;
		fprintf(logfile, "Process Killed %d\n", userMessage.userPID);
		fprintf(logfile, "waiting for alarm\n");
		//checkClock();
	}
	printf("final clock: %d\n", OSSclk->nanoSec);
	fprintf(logfile, "Cleaning up\n");
		
/*	pid_t pids[maxproc];
	for(i=0; i< maxproc; i++){
		if((pids[i] = fork()) < 0){
			perror("fork");
			return 1;
		}
		else if(pids[i] == 0){ //if in forked process
			printf("user process created %ld\n", getpid());
			execl("./user", "./user", NULL);
			exit(0);
		}
		else{
			(void)waitpid(pids[i], &status, 0);
		}
	}
*/
	releaseMem();
	sleep(1);
	fprintf(logfile, "oss done\n");
	fclose(logfile);
return 0;
}
//*********************************Create/attach to shared mem*************************************
void createShm(){
	//create message queue
	shm_msgQueue = msgget(msgKey, IPC_CREAT | 0666);
	if(shm_msgQueue < 0){
		perror("msgget");
		exit(1);
	}
	fprintf(logfile, "msg queue created\n");

	//get shared memory segment for clock
	shm_clock = shmget(shmkey1, sizeof(sysClock), IPC_CREAT | 0666);
	if(shm_clock < 0){
		perror("shmget failed");
		exit(1);	
	}

	//attach clock to shared memory
	OSSclk = (sysClock *)shmat(shm_clock,NULL,0);
	if(OSSclk->nanoSec == (int) -1){
		perror("shmat error on clock");
	}
	fprintf(logfile, "clock initialized\n");
	//printf("clock nano sec %d\n", OSSclk->nanoSec);
	//OSSclk->nanoSec = OSSclk->nanoSec + 10;
	//printf("clock add test %d\n", OSSclk->nanoSec);

	//get shared memory for maxproc
	shm_maxproc = shmget(shmkey3, sizeof(int), IPC_CREAT | 0666);
	if(shm_maxproc < 0){
		perror("shmget failed on maxproc");
		exit(1);
	}

	//attach procs to shared mem segment
	procs = (int*) shmat(shm_maxproc,NULL,0);
	//printf("procs attached\n");

	//semaphore setup
	shm_semaphore = semget(semKey, 1, IPC_CREAT | 0666);
	if(shm_semaphore < 0){
		perror("semaphore fail");
		exit(1);
	}	

}

//*****************************************Detach Mem**********************************************
void releaseMem(){
	//detach clock from shared memory
	int clockRelease = shmdt(OSSclk);
	if(clockRelease < 0){
		perror("shmdt fail on releasing clock var");
	}
	//delete/release shared memory 
	int clockDeleteError = shmctl(shm_clock, IPC_RMID, NULL);
	if(clockDeleteError < 0){
		perror("shmctl error on clock var\n");
	}
	
	//detach maxprocs
	int procsrelease = shmdt(procs);
	if(procsrelease < 0){
		perror("shmdt fail on releasing procs");
	}

	//release shared mem for maxprocs
	int procsDeleteError = shmctl(shm_maxproc, IPC_RMID, NULL);
	if(procsDeleteError < 0){
		perror("shmctl error on maxprocs");
	}

	//release message queue
	int msgQError = msgctl(shm_msgQueue, IPC_RMID, NULL);
	if(msgQError < 0){
		perror("msg queue release error\n");
	}

	//release semaphore
	int semError = semctl(shm_semaphore, 0, IPC_RMID);
	if(semError < 0){
		perror("semaphore removal error\n");
	}

}

void checkClock(){
	if(OSSclk->sec == (int) 2){
		setdoneflag((int)SIGALRM);
	}
}
