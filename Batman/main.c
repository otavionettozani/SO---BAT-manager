//
//  main.c
//  Batman
//
//  Created by Otávio Netto Zani on 08/04/15.
//  Copyright (c) 2015 Otávio Netto Zani. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


//---------------------------------------Definicoes-----------------------//
typedef enum{
	NONE = -1,
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3
}Directions;

//o cruzamento i eh o cruzamento entre a direcao i e a i+1
typedef enum crossings{
	NORTH_EAST = 0,
	SOUTH_EAST = 1,
	SOUTH_WEST = 2,
	NORTH_WEST = 3
}Crossings;

typedef struct arguments{
	
	int laneType;
	int number;
	char* busyFlag;
	char* crossings;
	pthread_cond_t* conditions;
	pthread_mutex_t* mutexes;
	
}Arguments;

typedef struct queue{
	
	int number;
	struct queue* next;
	
}Queue;


//---------------------------------------Funcoes Lista-----------------------//
//adiciona um elemento a fila
Queue* addElement(Queue* queue){
	
	Queue* newElement = (Queue*) malloc(sizeof(Queue));
	newElement->next = queue;

	return newElement;
}


//remove um elemento da fila
Queue* removeElement(Queue* queue){
	
	if(queue->next == NULL){
		free(queue);
		return NULL;
	}
	
	queue->next = removeElement(queue->next);
	
	return queue;
}

int queueLastNumber(Queue* queue){
	
	if(queue->next == NULL){
		return queue->number;
	}
	
	return queueLastNumber(queue->next);
	
}

//conta os elementos da queue
int queueCount(Queue* queue){
	
	if(queue == NULL){
		return 0;
	}
	
	return 1+queueCount(queue->next);
}


//---------------------------------------Funcoes Threads-----------------------//
char hasAMorePriorityThanB(int laneTypeA, int laneTypeB){
	
	return laneTypeB==0? laneTypeA==3 : laneTypeA>laneTypeB;
}

void waitSomeTime(){
	int i;
	
	for(i=0; i<100000; i++);
	
	return;
}


//funcao que sera executada pelas threads
void *BatBehaviour(void* args){

	//arguments processing
	Arguments* arguments = args;
	char directions[4] = {'N','E','S','W'};
	int rightLane = arguments->laneType == 3? 0:arguments->laneType+1;
	int leftLane = arguments->laneType == 0? 3:arguments->laneType-1;
	
	//a thread esta sendo ocupada
	arguments->busyFlag[0] = 1;
	printf("BAT %d %c chegou no cruzamento\n",arguments->number,directions[arguments->laneType]);

	switch (arguments->laneType) {
		case NORTH:
			pthread_mutex_lock(&arguments->mutexes[NORTH_EAST]);
			
		break;
		
		case SOUTH:
			
		break;
		
		case EAST:
			
		break;
			
		case WEST:
			
		break;
			
			
		default:
		break;
	}
	
	
	//a thread foi liberada
	arguments->busyFlag[0] = 0;
	
	pthread_exit((void*) args);
}


//---------------------------------------Main-----------------------//
int main(int argc, const char * argv[]) {
	
	//declaracao de variaveis genericas e de processamento de entrada
	int i;
	char lane;
	
	//declaracao de variaveis relacionadas a queues
	Queue **queues = (Queue**)malloc(4*sizeof(Queue*));
	
	//declaracao de variaveis relacionadas a threads e sua sincronizacao
	pthread_t threads[4] = {NULL, NULL, NULL, NULL};
	char busyFlags[4] = {0,0,0,0};
	Arguments arguments[4];
	pthread_attr_t attributes;
	
	
	//mutexes e condicionais
	char crossings[4] = {-1,-1,-1,-1};
	pthread_mutex_t crossingMutex[4];
	pthread_cond_t crossingCond[4];
	
	
	
	
	//inicializacao das variaveis de queues dos BATs.e dos mutexes e condicionais
	for (i=0; i< 4; i++) {
		queues[i] = NULL;
		pthread_mutex_init(&crossingMutex[i], NULL);
		pthread_cond_init(&crossingCond[i], NULL);
	}
	
	
	//interpretacao da entrada e populacao das queues
	if(argc != 2){
		printf("Numero invalido de argumentos, esperado 2, dados %d",argc);
		exit(1);
	}

	i=0;
	while ( (lane = argv[1][i]) ) {
		switch (lane) {
			
			case 'n':
				queues[NORTH] = addElement(queues[NORTH]);
				queues[NORTH]->number = i+1;
			break;
				
			case 's':
				queues[SOUTH] = addElement(queues[SOUTH]);
				queues[SOUTH]->number = i+1;
			break;
				
			case 'e':
				queues[EAST] = addElement(queues[EAST]);
				queues[EAST]->number = i+1;
			break;
				
			case 'w':
				queues[WEST] = addElement(queues[WEST]);
				queues[WEST]->number = i+1;
			break;
				
				
			default:
			break;
		}
		
		i++;
	}
	

	
	
	//setar os atributos da queue
	pthread_attr_init(&attributes);
	pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_JOINABLE);
	
	//setup inicial dos argumentos que serao passados para as funcoes
	for(i=0; i<4; i++){
		arguments[i].busyFlag = &busyFlags[i];
		arguments[i].laneType = i;
		arguments[i].crossings = crossings;
		arguments[i].conditions = crossingCond;
		arguments[i].mutexes = crossingMutex;
	}
	
	
	i=0;
	while (1) {

		// verifica se exite uma thread sendo executada para fila i
		if(!threads[i]){
			//se ainda forem necessaarias novas threads (a fila i nao estiver vazia), essa thread e criada
			if(queueCount(queues[i])){
				arguments[i].number = queueLastNumber(queues[i]);
				queues[i] = removeElement(queues[i]);
				if(pthread_create(&threads[i], &attributes, BatBehaviour, &arguments[i])){
					printf("erro ocorrido na criacao da thread");
				}
			}
			
		}
		else{
			
			//verifica se a thread ja terminou de executar para poder liberar novamente seu tid para ser usado por uma nova thread
			if(!busyFlags[i]){
				pthread_join(threads[i], NULL);
				threads[i] = NULL;
			}
			
		}
		
		
		
		//verifica se todas as queues ja se acabaram, em caso positivo aguarda ate o ultimo trem fazer o seu trajeto
		int remainingQueues = queueCount(queues[NORTH]);
		remainingQueues+= queueCount(queues[SOUTH]);
		remainingQueues+= queueCount(queues[EAST]);
		remainingQueues+= queueCount(queues[WEST]);
		
		
		if(remainingQueues == 0){
			pthread_join(threads[NORTH], NULL);
			pthread_join(threads[SOUTH], NULL);
			pthread_join(threads[EAST], NULL);
			pthread_join(threads[WEST], NULL);
			break;
		}
		
		//itera sobre a lista de threads
		i = i<3?i+1:0;
	}
	

	
	pthread_attr_destroy(&attributes);
    pthread_exit(NULL);
	
}
