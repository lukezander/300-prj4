#ifndef __KLOCK_H__
#define __KLOCK_H__
#include <pthread.h>

typedef struct {
	 pthread_mutex_t mutex;
	 int mutexID;
	 long int threadID;
} SmartLock;

typedef struct {
	long int threadID;
	int mutexID;
}ThreadLock_t;

typedef struct {
    SmartLock** mutexes;
    int capacity;
    int mutexCount;
} MutexCollection_t;

typedef struct {
    ThreadLock_t** threads;
    int capacity;
    int threadCount;
} ThreadCollection_t;

typedef struct {
    ThreadCollection_t* threadCollect;
    MutexCollection_t* mutexCollect;
} ResGraph_t;

void init_lock(SmartLock* lock);
int lock(SmartLock* lock);
void unlock(SmartLock* lock);
void cleanup();
ResGraph_t* init_graph();
void addMutex(MutexCollection_t *mList, SmartLock* lock);
int findThread(ThreadCollection_t* tList);

#endif
