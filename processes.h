//  author   : Kostiantyn Krukhmalov
//  login    : xkrukh00
//  describe : BUT FIT IOS 2. project - process synchronization
//  header file

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

#ifndef _PROCESSES_H

#define outputImm(ACTION, incNE, incNC, incNB) sem_wait(semWrOut);\
file = fopen("proj2.out", "a");\
fprintf(file,"%d\t: IMM %d\t\t: %s\t: %d\t: %d\t: %d\n", ++strcp->A, immID, ACTION, strcp->NE+=incNE, strcp->NC+=incNC, strcp->NB+=incNB);\
fclose(file);\
sem_post(semWrOut)

#define outputJudge(ACTION) sem_wait(semWrOut);\
file = fopen("proj2.out", "a");\
fprintf(file,"%d\t: JUDGE\t\t: %s\t: %d\t: %d\t: %d\n", ++strcp->A, ACTION, strcp->NE, strcp->NC, strcp->NB);\
fclose(file);\
sem_post(semWrOut)

typedef struct {
	unsigned int A;  /* pořadové číslo prováděné akce */
	unsigned int PI; /* pocet processu immigrantu. PI >= 1 */
	unsigned int IG; /* max doba po ktere bude generovan immigrant <0,2000> */
	unsigned int JG; /* max doba po ktere soudce opet vstoupi do budovy <0,2000> */
	unsigned int IT; /* max doba trvani vyzvedani certifikatu <0,2000> */
	unsigned int JT; /* max doba rozhodnuti soudcem <0,2000> */
	unsigned int NB; /* pocet immigrantu v budove */
	unsigned int NE; /* pocet immigrantu v budove, a nebylo o nich rozhodnuto */
	unsigned int NC; /* pocet immigrantu v budove, ktere se zaregistovali a nebylo o nich rozhodnuto */
	unsigned int PD; /* pocet immigrantu kteri dostali certificat */
	}shStrc_t;

void randomDelay(unsigned int delay){
	srand(time(NULL));
	if(delay > 0){
		usleep((rand()%delay)*1000);
	}
}

void procImm(unsigned int immID, shStrc_t *strcp, sem_t *semEntry, sem_t *semCheck, sem_t *semConfirm, sem_t *semExit, sem_t *semWrOut) {
	if(fork()==0) {
        	/* Immigrant code */
		FILE *file;

		sem_wait(semWrOut);
		file = fopen("proj2.out", "a");
		fprintf(file,"%d\t: IMM %d\t\t: starts\n", ++strcp->A, immID);
		fclose(file);
		sem_post(semWrOut);

        	sem_wait(semEntry); /* ENTER */
		outputImm("enters	", 1, 0, 1);
		sem_post(semEntry);
		
		sem_wait(semCheck); /* CHECK IN */
		outputImm("checks	", 0, 1, 0);
		sem_post(semCheck); 
		
		sem_wait(semConfirm); /* WAIT FOR JUDGE`S DECISION*/
		outputImm("wants certificate", 0, 0, 0);
		randomDelay(strcp->IT);
		outputImm("got certificate", 0, 0, 0);
		sem_post(semConfirm);
		
		sem_wait(semExit); /* LEAVE */
		outputImm("leaves	", 0, 0, -1);
		sem_post(semExit);

        	exit(0);
	}
}

void procJudge(shStrc_t *strcp, sem_t *semEntry, sem_t *semConfirm, sem_t *semExit, sem_t *semWrOut) {
   	
	FILE *file;
	while(strcp->PD != strcp->PI) {
	
		/* Decrement all semaphores */
		sem_wait(semConfirm); // nobody can pass the confirmation without judge
	
		randomDelay(strcp->JG);
		sem_wait(semWrOut);
		file=fopen("proj2.out", "a");
  		fprintf(file, "%d\t: JUDGE\t\t: wants to enter\n", ++strcp->A);
		fclose(file);
		sem_post(semWrOut);
		
		// ENTER
		sem_wait(semExit); 	// Closing exit
    		sem_wait(semEntry);	// and enter
		outputJudge("enters	");
 	
		// WAIT FOR IMMIGRANTS
		if (strcp->NE != strcp->NC) {
			outputJudge("waits for imm	");
			while (strcp->NE != strcp->NC){}
		}

 		// CONFIRM
		outputJudge("starts confirmation");
		randomDelay(strcp->JT);
		strcp->PD += strcp->NE;
		strcp->NE = 0;
		strcp->NC = 0;
		outputJudge("ends confirmation");
		sem_post(semConfirm);

		// EXIT
   		randomDelay(strcp->JG);
		outputJudge("leaves	");
		sem_post(semExit); // Immigrants can leave the building
		sem_post(semEntry);
	}
	sem_wait(semWrOut);
	file=fopen("proj2.out", "a");
  	fprintf(file, "%d\t: JUDGE\t\t: finishes\n",++strcp->A);
	fclose(file);
	sem_post(semWrOut);
	fclose(file);
	exit(0);
}

#endif /* processes.h */
