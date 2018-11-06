#include "klock.h"
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>



sem_t lock;
static bool opened = false;
static ResGraph_t* RAG;

ResGraph_t* init_graph() {
    ResGraph_t* RAG = malloc(sizeof(ResGraph_t));
    MutexCollection_t* mList = malloc(sizeof(MutexCollection_t));
    ThreadCollection_t* tList = malloc(sizeof(ThreadCollection_t));
    sem_init(&lock, 0, 1);

    //default to 10
    mList->capacity = 10;
    mList->mutexCount = 0;
    mList->mutexes = calloc(mList->capacity, sizeof(SmartLock*));

    //default to 10
    tList->capacity = 10;
    tList->threadCount = 0;
    tList->threads = calloc(tList->capacity, sizeof(ThreadLock_t**));

    RAG->mutexCollect = mList;
    RAG->threadCollect = tList;

    return RAG;
}

void addMutex(MutexCollection_t *mList, SmartLock* lock) {
    sem_wait(&lock);
    if (mList->capacity <= mList->mutexCount) {
        SmartLock** newMutexes = calloc(mList->capacity * 2, sizeof(SmartLock*));
        int i=0;
        while(i < mList->mutexCount){
            newMutexes[i] = mList->mutexes[i];
            i++;
        }


        mList->mutexes = newMutexes;
        mList->capacity *= 2;
    }

    lock->mutexID = mList->mutexCount; //unique ID to mutex
    mList->mutexes[mList->mutexCount] = lock; //add mutex to array
    mList->mutexCount++;
    sem_post(&lock);
}

void addThread(ThreadCollection_t* tList) {

    //increases size of array
    if (tList->capacity <= tList->threadCount) {
        ThreadLock_t** nThreads = calloc(tList->capacity * 2, sizeof(ThreadLock_t*));
        int i = 0;
        while(i < tList->threadCount){
            nThreads[i] = tList->threads[i];
            i++;
        }

        tList->threads = nThreads;
        tList ->capacity *= 2;
    }

    //default values for new thread
    ThreadLock_t* thread = malloc(sizeof(ThreadLock_t));
    thread->threadID = pthread_self();
    thread->mutexID = -1;

    //add thread to threads array
    tList->threads[tList->threadCount] = thread; 
    tList->threadCount++;
    
} 

int findThread(ThreadCollection_t* tList) {
    unsigned long int origThreadID = pthread_self();
    int i=0;
    while(i < tList->threadCount) {
        if (origThreadID == tList->threads[i]->threadID)
            return i;
        else{
            i++;
        }
    }

    return -1;
}

void init_lock(SmartLock* lock) {
    pthread_mutex_init(&(lock->mutex), NULL);
    lock->threadID = 0;

    if(!opened) {
        sem_init(&lock, 0, 1);
        RAG = init_graph(lock);
        opened = true;
    }

    //dynamically increases size of array
    addMutex(RAG->mutexCollect, lock);
}

int lock(SmartLock* lock) {
    printf("%lu locking\n", pthread_self());
    sem_wait(&lock);

    ThreadCollection_t *tList = RAG->threadCollect;


    int threadIndex = findThread(tList);
    sem_post(&lock);

        if (pthread_mutex_trylock(&(lock->mutex)) > 0) {
            tList->threads[threadIndex]->mutexID = lock->mutexID;
            return 0;

        } else {
            lock->threadID = pthread_self();
            return 1;
        }
    
}

void unlock(SmartLock* lock) {
    pthread_mutex_unlock(&(lock->mutex));
    lock->threadID = 0;
}

/*
 * Cleanup any dynamic allocated memory for SmartLock to avoid memory leak
 * You can assume that cleanup will always be the last function call
 * in main function of the test cases.
 */
void cleanup() {
    MutexCollection_t* mList = RAG->mutexCollect;
    ThreadCollection_t *tList = RAG->threadCollect;

    for(int i = 0; i < mList->mutexCount; i++)
        pthread_mutex_destroy(&(mList->mutexes[i]->mutex));

    for(int i =0; i < tList->threadCount; i++)
        free(tList->threads[i]);

    free(RAG);

}