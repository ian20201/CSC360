#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define BILLION 1E9;
pthread_mutex_t mutex;
pthread_mutex_t crosstex;
pthread_mutex_t crosstex4;
pthread_mutex_t waitex;
pthread_cond_t loadCond;
pthread_cond_t crossCond;
pthread_cond_t dispatch;
int init = 0;
int counter = 0;
int trainCount = 0;
int waiting = 0;
int wait2 = 0;
int test2 = 0;
int crossing = 0;
int flag[2] = {0,0};
int turn;
char lastDir = 'a';

struct t{
	char dir; // west or east
	int priority; //1 = high, 0 = low
	int load;
	int cross;
	int status; //1 = ready, 9 = not ready
	int num;
	pthread_cond_t access;
	struct t *next;
}*trains;

struct t *westP = NULL;
struct t *westL = NULL;
struct t *eastP = NULL;
struct t *eastL = NULL;

//typedef struct t *Train;

void *train(void *line){
	struct timespec start, stop, stop2, stop3;
	double accum;//sec
	int hour;
	int min;

	struct t *tst = (struct t*)line;
	pthread_mutex_lock(&mutex);
	while(init == 0){
		counter++;
		//printf("%d\n", counter);
		pthread_cond_wait(&loadCond, &mutex);
	}
	
	pthread_mutex_unlock(&mutex);
	if(clock_gettime(CLOCK_REALTIME, &start) == -1){
		perror("Clock GetTime");
		exit(EXIT_FAILURE);
	}
	usleep(tst->load*100000);
	if(clock_gettime(CLOCK_REALTIME, &stop) == -1){
		perror("Clock GetTime");
		exit(EXIT_FAILURE);
	}
	accum = (stop.tv_sec -start.tv_sec)+(stop.tv_nsec-start.tv_nsec)/BILLION;
	hour = accum/3600;
	min = accum/60;
	pthread_mutex_lock(&crosstex4);
	flag[0] = 1;
	turn = 1;
	while(flag[1] == 1 && turn == 1);
	if(tst->dir == 'w' || tst->dir == 'W'){
		printf("%02d:%02d:%04.1lf Train %d is ready to go West\n", hour, min,accum, tst->num);
		if(tst->dir == 'w'){
			if(westL == NULL){
				westL = tst;
			}else{
				westL->next = tst;
			}
		}else{
			if(westP == NULL){
				westP = tst;
			}else{
				westP->next = tst;
			}
		}
	}else{
		printf("%02d:%02d:%04.1lf Train %d is ready to go East\n",hour, min, accum, tst->num);
		if(tst->dir == 'e'){
			if(eastL == NULL){
				eastL = tst;
			}else{
				eastL->next = tst;
			}
		}else{
			if(eastP == NULL){
				eastP = tst;
			}else{
				eastP->next = tst;
			}
		}
	}
	flag[0] = 0;
	
	pthread_mutex_unlock(&crosstex4);

	pthread_mutex_lock(&crosstex);
	
	pthread_cond_signal(&dispatch);

	while(tst->status != 1){
		//printf("2Train#: %d\n", tst->num);
		pthread_cond_wait(&tst->access,&crosstex);
		//printf("Test: %d\n", tst->num);
	}
	//printf("Train#: %d\n", tst->num);
	if(clock_gettime(CLOCK_REALTIME, &stop2) == -1){
		perror("Clock GetTime");
		exit(EXIT_FAILURE);
	}
	
	accum = (stop2.tv_sec -start.tv_sec)+(stop2.tv_nsec-start.tv_nsec)/BILLION;
	hour = accum/3600;
	min = accum/60;

	//while(tst->light == 0){
	//	pthread_cond_wait(&tst->access,&crosstex);
	//}
	if(tst->dir == 'w' || tst->dir == 'W'){
		printf("%02d:%02d:%04.1lf Train %d is on the main track going West\n", hour, min, accum ,tst->num);
	}else{
		printf("%02d:%02d:%04.1lf Train %d is on the main track going East\n", hour,min, accum ,tst->num);
	}

	usleep(tst->cross*100000);
	if(clock_gettime(CLOCK_REALTIME, &stop3) == -1){
		perror("Clock GetTime");
		exit(EXIT_FAILURE);
	}

	accum = (stop3.tv_sec -start.tv_sec)+(stop3.tv_nsec-start.tv_nsec)/BILLION;
	hour = accum/3600;
	min = accum/60;

	if(tst->dir == 'w' || tst->dir == 'W'){
		printf("%02d:%02d:%04.1lf Train %d is OFF the main track after going West\n", hour, min,accum ,tst->num);
	}else{
		printf("%02d:%02d:%04.1lf Train %d is OFF the main track after going East\n", hour, min,accum ,tst->num);
	}
	pthread_cond_signal(&dispatch);
	pthread_mutex_unlock(&crosstex);
	
	//printf("Test: %c  %d  %d  %d %lf\n", tst->dir, tst->priority, tst->load, tst->cross, accum);
	//pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]){

	char line[256];
	int tracker = 0;
	
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&crosstex, NULL);
	pthread_mutex_init(&crosstex4, NULL);
	pthread_mutex_init(&waitex, NULL);
	pthread_cond_init(&loadCond, NULL);
	pthread_cond_init(&crossCond, NULL);
	
	if(argc != 2){
		printf("Please enter exactly 1 Train File\n");
		exit(1);
	}
	
	FILE *textFile = fopen(argv[1], "r");
	
	if(textFile == NULL){  
		printf("File %s not valid\n", argv[1]);
		exit(1);
	}
	
	while(fgets(line, sizeof(line), textFile)){
		trainCount++;
	}
	
	pthread_t tid[trainCount];
	rewind(textFile);
	int priority;
	
	while(fgets(line, sizeof(line), textFile)){
		//printf("%s", line);
		//printf("%d\n",trainCount);
		trains = (struct t*)malloc(sizeof(struct t));
		char *p = strtok(line, " ");
		trains->dir = *p;
		
		if(trains->dir == 'E' || trains->dir == 'W'){
			trains->priority = 1;
		}else{
			trains->priority = 0;
		}
		
		p = strtok(NULL, " ");
		trains->load = atoi(p);
		p = strtok(NULL, " ");
		trains->cross = atoi(p);
		trains->status = 0;
		trains->num = tracker;
		pthread_cond_init(&trains->access, NULL);
		pthread_create(&tid[tracker], NULL, train, (void *)trains);

		tracker++;
	}
	
	while(counter != trainCount){
	}
	
	init = 1;
	pthread_cond_broadcast(&loadCond);

	pthread_mutex_lock(&crosstex);
	while(crossing < trainCount){
	
		pthread_cond_wait(&dispatch,&crosstex);
		//printf("Testing\n");
		//if queues arent empty

		flag[1] = 1;
		turn = 0;
		while(flag[0] == 1 && turn == 0);
		if(eastL != NULL || eastP != NULL || westL != NULL || westP != NULL){
			//if a priority queue isn't empty
			if(eastP != NULL || westP != NULL){
				//if no trains are crossed yet
				if(crossing == 0){
					//if eastP not empty, east gets priority
					if(eastP != NULL){
						eastP->status = 1;
						pthread_cond_signal(&eastP->access);
						eastP = eastP->next;
						lastDir = 'e';
						crossing++;
					}
					//eastp is empty so west goes first
					else{
						westP->status = 1;
						pthread_cond_signal(&westP->access);
						westP = westP->next;
						lastDir = 'w';
						crossing++;
					}
				//if trains previously crossed
				}else{
					//if last train went west
					if(lastDir = 'w'){
						//if eastP not empty
						if(eastP != NULL){
							eastP->status = 1;
							pthread_cond_signal(&eastP->access);
							eastP = eastP->next;
							lastDir = 'e';
							crossing++;
						//westP isn't empty
						}else{
							westP->status = 1;
							pthread_cond_signal(&westP->access);
							westP = westP->next;
							lastDir = 'w';
							crossing++;
						}
					//last train went east
					}else{
						//if westP not empty
						if(westP != NULL){
							westP->status = 1;
							pthread_cond_signal(&westP->access);
							westP = westP->next;
							lastDir = 'w';
							crossing++;
						//eastP isn't empty
						}else{
							eastP->status = 1;
							pthread_cond_signal(&eastP->access);
							eastP = eastP->next;
							lastDir = 'e';
							crossing++;
						}
					}
				}
			}
			else if ((westP == NULL && eastP == NULL) && westL != NULL || eastL != NULL){
				//if a priority queue isn't empty
				if(eastL != NULL || westL != NULL){
					//if no trains are crossed yet
					if(crossing == 0){
						//if eastl not empty, east gets priority
						if(eastL != NULL){
							eastL->status = 1;
							pthread_cond_signal(&eastL->access);
							eastL = eastL->next;
							lastDir = 'e';
							crossing++;
						}
						//eastl is empty so west goes first
						else{
							westL->status = 1;
							pthread_cond_signal(&westL->access);
							westL = westL->next;
							lastDir = 'w';
							crossing++;
						}
					//if trains previously crossed
					}else{
						//if last train went west
						if(lastDir = 'w'){
							//if eastl not empty
							if(eastL != NULL){
								eastL->status = 1;
								pthread_cond_signal(&eastL->access);
								eastL = eastL->next;
								lastDir = 'e';
								crossing++;
							//westl isn't empty
							}else{
								westL->status = 1;
								pthread_cond_signal(&westL->access);
								westL = westL->next;
								lastDir = 'w';
								crossing++;
							}
						//last train went east
						}else{
							//if westl not empty
							if(westL != NULL){
								westL->status = 1;
								pthread_cond_signal(&westL->access);
								westL = westL->next;
								lastDir = 'w';
								crossing++;
							//eastl isn't empty
							}else{
								eastL->status = 1;
								pthread_cond_signal(&eastL->access);
								eastL = eastL->next;
								lastDir = 'e';
								crossing++;
							}
						}
					}
				}
			}

		}
		flag[1] = 0;
	}
	pthread_mutex_unlock(&crosstex);

	//printf("done");
	int i = 0;
	for(i = 0; i < trainCount; i++){
		pthread_join(tid[i], NULL);
	}

	return 0;
}
