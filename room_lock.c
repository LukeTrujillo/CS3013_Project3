#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

typedef struct _room_lock_t {
	int max;
	int occupants;
	sem_t room_lock;
	sem_t lock;
} room_lock_t;

void room_lock_init(room_lock_t *rl, unsigned int teams) {
	rl->max = teams;
	rl->occupants = 0;
	sem_init(&rl->room_lock, 0, teams); // allow the number of teams to enter the room
	sem_init(&rl->lock, 0, 1);
}


void room_lock_acquire_lock(room_lock_t *lock) {
	sem_wait(&lock->lock); //only one thread at a time can enter here
	lock->occupants++;

	if(lock->occupants > lock->max) {
		printf("Max capacity for room achieved...all threads calling room_lock_acquire_room_lock() will be waiting\n");
		sem_wait(&lock->room_lock);
	}
	
	printf("lock acquired occup %d", lock->occupants);
	
	sem_post(&lock->lock);
}
void room_lock_release_lock(room_lock_t *lock) {
	sem_wait(&lock->lock); //only one thread can enter here
	lock->occupants--;

	if(lock->occupants == 0) {
		sem_post(&lock->room_lock);
	}
	
	printf("lock released occupants: %d\n", lock->occupants);

	sem_post(&lock->lock);
}

unsigned int atMaxOccupancy(room_lock_t *lock) {
	unsigned int max;

	sem_wait(&lock->lock); //only one thread at a time
	max = lock->max;
	sem_post(&lock->lock);

	return max;
}
