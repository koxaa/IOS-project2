//  author   : Kostiantyn Krukhmalov
//  login    : xkrukh00
//  describe : BUT FIT IOS 2. project - process synchronization
//  source file

#include "processes.h"

#define	free_all_allocated_memory sem_close(sem1);\
	sem_unlink("/sem1");\
	sem_close(sem2);\
	sem_unlink("/sem2");\
	sem_close(sem3);\
	sem_unlink("/sem3");\
	sem_close(sem4);\
	sem_unlink("/sem4");\
	sem_close(sem5);\
	sem_unlink("/sem5");\
	munmap(sptr, sizeof(shStrc_t));\
	shm_unlink("/sharedmem1");\
	close(fd)

/*	******************* VSTUP *****************

	$ ./proj2 PI IG JG IT JT

 PI;        pocet processu immigrantu. PI >= 1
 IG;        max doba po ktere bude generovan immigrant <0,2000>
 JG;        max doba po ktere soudce opet vstoupi do budovy <0,2000>
 IT;        max doba trvani vyzvedani certifikatu <0,2000>
 JT;        max doba rozhodnuti soudcem <0,2000>

	*******************************************/

shStrc_t *sptr;
int fd;
pid_t pidImmGen, pidJudge;
sem_t *sem1, *sem2, *sem3, *sem4, *sem5;
/*	sem1 for entry, sem2 for check in, 
	sem3 for confirmation, sem4 for exit	
	sem5 for writing output		*/

int main(int argc, char **argv) {
/******** Inputs Checks *********/
	if(argc != 6) {
		perror("Invalid number of arguments");
		fprintf(stderr, "$ %s PI IG JG IT JT\n", argv[0]);
		return EXIT_FAILURE;
	}
	for (int i = 1; i < argc; i++){
		if (atoi(argv[i]) < 0 || atoi(argv[i]) > 2000){
			perror("Invalid value of argument(s). Must be in range <0,2000>");
			return EXIT_FAILURE;
		}
	}

/********* Cleaning file for output **********/
	FILE *file;	
	file=fopen("proj2.out", "w+");
	if (file == NULL) {
		return EXIT_FAILURE;
		perror("error : fopen() : Couln't open open/create file for output");
	}
	fclose(file);

/********* Creat shared object and set its size *********/
	fd=shm_open("/sharedmem1", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("error : shm_open()");
		return EXIT_FAILURE;
	}
	if (ftruncate(fd, sizeof(shStrc_t)) == -1) {
		perror("error : ftruncate()");
		close(fd);
		return EXIT_FAILURE;
	}

/********* Map shared memory object **********/
	sptr = mmap(NULL, sizeof(shStrc_t), PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if (sptr == MAP_FAILED) {
		perror("error : mmap()");
		close(fd);
		return EXIT_FAILURE;
	}

/********* Semaphore initialize ***********/

        sem1 = sem_open("/sem1", O_CREAT | O_EXCL, 0644, 1);
        sem2 = sem_open("/sem2", O_CREAT | O_EXCL, 0644, 1);
        sem3 = sem_open("/sem3", O_CREAT | O_EXCL, 0644, 1);
        sem4 = sem_open("/sem4", O_CREAT | O_EXCL, 0644, 1);
        sem5 = sem_open("/sem5", O_CREAT | O_EXCL, 0644, 1);

        if (sem1 == SEM_FAILED || sem2 == SEM_FAILED || sem3 == SEM_FAILED || sem4 == SEM_FAILED || sem5 == SEM_FAILED ) {
		free_all_allocated_memory;	
		perror("error : sem_open()");

		return EXIT_FAILURE;
        }


/********* Process Creating ***********/
	
	sptr->PI = atoi(argv[1]);
	sptr->IG = atoi(argv[2]);
	sptr->JG = atoi(argv[3]);
	sptr->IT = atoi(argv[4]);
	sptr->JT = atoi(argv[5]);
	sptr->A = 0;
	sptr->NB=0;
	sptr->NE=0;
	sptr->NC=0;
	sptr->PD=0;

	pidImmGen = fork();
	if( pidImmGen == 0) {
		
		/******** Immigrants processes generator *********/
		for(unsigned int i = 0; i < sptr->PI; i++) {
			randomDelay(sptr->IG);
			procImm(i+1 ,sptr, sem1, sem2, sem3, sem4, sem5); // foo that generate an immigrant
		}
		for(unsigned int a = 0; a < sptr->PI; a++) {
			wait(NULL);
		}
		exit(0);		
	} else {
		pidJudge = fork();
		if (pidJudge == 0){
			
			/****** Judge Process ******/
			procJudge(sptr, sem1, sem3, sem4, sem5);
		} else {
			
			/****** Parent process ******/
			if (pidImmGen < 0 || pidJudge < 0) {
				kill(pidImmGen, SIGKILL);
				kill(pidJudge, SIGKILL);
				free_all_allocated_memory;
				exit(1);
			} else {
				for(int i = 0; i < 2; i++) { // Waiting for all processesi(with judge)
					wait(NULL);
				}
			}
			// Destroing all semaphores
			
			free_all_allocated_memory;
			exit(0);
		}
	}
}
