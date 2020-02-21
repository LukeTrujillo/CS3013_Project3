#include <pthread.h>
#include <time.h>
#include <semaphore.h>


typedef struct _team_lock_t {
	sem_t writeLock;
	sem_t lock;
	int readers;
} team_lock_t;

void team_lock_init(team_lock_t *rw) {
	rw->readers = 0;
	sem_init(&rw->lock, 0, 1);
	sem_init(&rw->writeLock, 0, 1);
}


void team_lock_acquire_readlock(team_lock_t *lock) {
    sem_wait(&lock->lock);
    lock->readers++;
    if (lock->readers == 1)
	sem_wait(&lock->writelock);
    sem_post(&lock->lock);
}

void team_lock_release_readlock(team_lock_t *lock) {
    sem_wait(&lock->lock);
    lock->readers--;
    if (lock->readers == 0)
	sem_post(&lock->writelock);
    sem_post(&lock->lock);
}

void team_lock_acquire_writelock(team_lock_t *lock) {
    sem_wait(&lock->writelock);
}

void team_lock_release_writelock(team_lock_t *lock) {
    sem_post(&lock->writelock);
}
