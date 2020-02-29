#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

enum Team {NINJAS, PIRATES};

struct ArrivalNode {
		struct ArrivalNode *next;
		unsigned int arrivalTime;
		unsigned int thread_id;
};

struct CostumeTeam {
	unsigned int serving;
	unsigned int currentEntity;
	
	unsigned teamNumber;
	unsigned int timeSpentBusy;
};

struct Visit {
	struct ArrivalNode *arrival;
	
	unsigned int timeSpent;
	unsigned int entity;
	
	enum Team entityTeam;

	struct CostumeTeam *servicedBy;
	struct Visit *next;
};


typedef struct _room_lock_t {
	int max;
	
	sem_t room_lock;
	sem_t lock;
	
	enum Team currentTeam;
	unsigned int swapTrigger;
	
	struct CostumeTeam *costumeTeams;
	struct Visit *head;
	
} room_lock_t;


unsigned int getOccupancy(room_lock_t *);
unsigned int atMaxOccupancy(room_lock_t*);
void occupy(room_lock_t*, struct Visit*);
void exitRoom(room_lock_t*, struct Visit *);

void room_lock_init(room_lock_t *rl, unsigned int teams) {
	rl->max = teams;
	sem_init(&rl->room_lock, 0, teams); // allow the number of teams to enter the room
	sem_init(&rl->lock, 0, 1);
	
	
	rl->costumeTeams = ((struct CostumeTeam*) malloc(sizeof(struct CostumeTeam) * teams));
	rl->swapTrigger = 0;
	
	
	for(int x = 0; x < teams; x++) {
		rl->costumeTeams[x].serving = 0;
		rl->costumeTeams[x].teamNumber = x;
		rl->costumeTeams[x].timeSpentBusy = 0;		
	}
	
	rl->head = NULL;

	srand(time(NULL));
	
	unsigned int choice = rand() % 2;
	
	if(choice) {
		 rl->currentTeam = PIRATES;
	}
	else {
		 rl->currentTeam = NINJAS;
	}


}


unsigned int room_lock_will_accept(room_lock_t *lock, enum Team team) {
	return !atMaxOccupancy(lock) && ((team == lock->currentTeam && lock->swapTrigger == 0) || (getOccupancy(lock) == 0 && lock->swapTrigger == 1 && lock->currentTeam != team)); //will need to change later for dynamic switching
}


void room_lock_acquire_lock(room_lock_t *lock, struct Visit *visit) {
	sem_wait(&lock->lock);

	if(visit->entityTeam != lock->currentTeam) { //it should not wait for the lock
		//we can check if the thing would finish before
		
		sem_post(&lock->lock);
		return;
	}
	
	//it should wait for the lock

	if(atMaxOccupancy(lock)) {
		//printf("Max capacity for room achieved...all threads calling room_lock_acquire_room_lock() will be waiting\n");
		
		sem_wait(&lock->room_lock);
	}
	sem_post(&lock->lock);
	
	occupy(lock, visit);
}

void room_lock_set_team(room_lock_t *lock, enum Team team) {
	sem_wait(&lock->lock);
		
		if(lock->swapTrigger == 1 && lock->currentTeam == team) lock->swapTrigger = 0;
	
		
		if(lock->currentTeam != team) { 
			lock->swapTrigger = 1;
		}
	sem_post(&lock->lock);
}

void room_lock_release_lock(room_lock_t *lock, struct Visit *visit) {
	
	sem_wait(&lock->lock); //only one thread can enter here

	exitRoom(lock, visit);
	
	if(getOccupancy(lock) == 0 && lock->swapTrigger == 1) {
		

			lock->swapTrigger = 0;
			lock->currentTeam = !lock->currentTeam;
			
			//printf("THE TEAM ALLOWED HAS CHANGED TO TEAM #%d\n", lock->currentTeam);
	}
	
	//printf("\tTeam #%d member #%d left the room after being serviced by costume team #%d\n", visit->entityTeam, visit->entity, visit->servicedBy->teamNumber);

	sem_post(&lock->lock);
}

unsigned int atMaxOccupancy(room_lock_t *lock) {
	return lock->max <= getOccupancy(lock);
}
unsigned int getOccupancy(room_lock_t *lock) {
	unsigned int occupants = 0;
	
	unsigned int max = lock->max;
	for(int x = 0; x < max; x++) {
		struct CostumeTeam *team = &lock->costumeTeams[x];
		
		if(team->serving == 1) occupants++;
	}
	
	return occupants;
}

void occupy(room_lock_t *lock, struct Visit *visit) {
	sem_wait(&lock->lock); //only one thread can enter here
	if(!atMaxOccupancy(lock)) {
		
		unsigned int occupants = 0;
		
		for(int x = 0; x < lock->max; x++) {
			struct CostumeTeam *team = &lock->costumeTeams[x];
		
		
			if(team->serving == 0) {
				team->serving = 1;
				
				team->currentEntity = visit->entity;
				team->timeSpentBusy += visit->timeSpent;
				
				visit->servicedBy = team;
				
				struct Visit *node = lock->head;
				
				if(lock->head == NULL) { lock->head = visit; }
				else {
						while(node->next != NULL) {node = node->next;}
					
						node->next = visit;
				} 
				
		
				if(visit->entityTeam == PIRATES) {
					printf("Pirate %d entered the shop with team %d\n", visit->entity, visit->servicedBy->teamNumber);	
				} else {
					printf("Ninja %d entered the shop with team %d\n", visit->entity, visit->servicedBy->teamNumber);		
				}
				
				
				break;
			}
		
		}
		
		if(visit->servicedBy == NULL) {
			//printf("called again -- \n");
				occupy(lock, visit);
		}
		
		
	}
	sem_post(&lock->lock); //only one thread can enter here
}
void exitRoom(room_lock_t *lock, struct Visit *visit) {
	
	if(visit->servicedBy != NULL) {
		if(visit->entityTeam == PIRATES) {
			printf("Pirate %d exit the shop with team %d\n", visit->entity, visit->servicedBy->teamNumber);	
		} else {
			printf("Ninja %d exit the shop with team %d\n", visit->entity, visit->servicedBy->teamNumber);		
		}
		
		visit->servicedBy->serving = 0;
	}
}
