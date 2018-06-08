#include "info.h"

/*typedef struct {
	long mtype =1;
	char mtext[100];
} mymsg_t;
*/

int main (int argc, char * argv[]){
	sleep(1);
	char buffer[100];
	mymsg_t messageToSend;
	messageToSend.mtype = 1;
	//printf("establish mtype: %d\n", messageToSend.mtype);

	//printf("in user process: %d\n", getpid());
	
	int getProc = shmget(shmkey3, sizeof(int), 0666);
	if(getProc < 0){
		perror("user shmget fail");
		exit(-1);
	} 
	/*
	else {
		fprintf(logfile,"user shmget successful\n");
	}
	*/
	int * usermaxproc = (int*)shmat(getProc, NULL,0);

	int getClock = shmget(shmkey1, sizeof(sysClock), 0666);

	sysClock * userClock = (sysClock *) shmat(getClock, NULL, 0);
	//printf("print clock from user %d\n", userClock->nanoSec);
	userClock->nanoSec = userClock->nanoSec +10;
	//printf("after add on clock: %d\n", userClock->nanoSec); 
	//printf("maxproc %d \n", *usermaxproc);
	//sleep(2);

	//get message queue and send message
	int msgQID = msgget(msgKey, 0666);
	if(msgQID < 0){
		perror("message queue fail in user");
		exit(-1);
	}
	//*usermaxproc = *usermaxproc +1;
	//printf("after add: %d\n", *usermaxproc);
	messageToSend.proc = *usermaxproc; 
	//printf("test transfer to struct: %d\n", messageToSend.proc);
	messageToSend.userPID = getpid();
	//printf("test pid in struct: %d\n", messageToSend.userPID);
	printf("before message\n");
	if(msgsnd(msgQID, (void*)&messageToSend, sizeof(mymsg_t), 0) == -1){
		perror("user messaged failed to send\n");
		exit(-1);
	}
	
	while(1){

	}
	printf("user process %d finished\n", getpid());
	return 0;

}
