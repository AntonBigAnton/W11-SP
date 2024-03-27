/*
 * BlockingQueue.c
 *
 * Fixed-size generic array-based BlockingQueue implementation.
 *
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include "BlockingQueue.h"

/*
 * The functions below all return default values and don't work.
 * You will need to provide a correct implementation of the BlockingQueue module interface as documented in BlockingQueue.h.
 */


BlockingQueue *new_BlockingQueue(int max_size) {
    // Initialise the blocking queue
    BlockingQueue* this = malloc(sizeof(BlockingQueue));

    // Initialise the blocking queue's Queue object and maximum capacity
    (*this).queue = new_Queue(max_size);
    (*this).capacity = max_size;

    // Initialise the blocking queue's mutexes, and check that they've been created properly
    if (pthread_mutex_init(&(*this).mutex_enq, NULL)) {
        exit_error(this, "Mutex 'mutex_enq' not created!");
    }
    if (pthread_mutex_init(&(*this).mutex_deq, NULL)) {
        exit_error(this, "Mutex 'mutex_deq' not created!");
    }

    // Initialise the blocking queue's semaphores, and check that they've been created properly
    if (sem_init(&(*this).sem_enq, ZERO, max_size)) {
        exit_error(this, "Semaphore 'sem_enq' not created!");
    }
    if (sem_init(&(*this).sem_deq, ZERO, ZERO)) {
        exit_error(this, "Semaphore 'sem_deq' not created!");
    }

    return this;
}

bool BlockingQueue_enq(BlockingQueue* this, void* element) {
    // Decrement the sem_enq semaphore and check that it has been done
    if (sem_wait(&(*this).sem_enq)) {
        exit_error(this, "Semaphore 'sem_enq' not decremented!");
    }
    // Lock the mutex_enq mutex and check that it has been done
    if (pthread_mutex_lock(&(*this).mutex_enq)) {
        exit_error(this, "Mutex 'mutex_enq' not locked!");
    }
    // Enqueue the element
    bool value = Queue_enq((*this).queue, element);
    // Unlock the mutex_enq mutex and check that it has been done
    if (pthread_mutex_unlock(&(*this).mutex_enq)) {
        exit_error(this, "Mutex 'mutex_enq' not unlocked!");
    }
    // Increment the sem_deq semaphore and check that it has been done
    if (sem_post(&(*this).sem_deq)) {
        exit_error(this, "Semaphore 'sem_deq' not incremented!");
    }
    return value;
}

void* BlockingQueue_deq(BlockingQueue* this) {
    // Decrement the sem_deq semaphore and check that it has been done
    if (sem_wait(&(*this).sem_deq)) {
        exit_error(this, "Semaphore 'sem_deq' not decremented!");
    }
    // Lock the mutex_deq mutex and check that it has been done
    if (pthread_mutex_lock(&(*this).mutex_deq)) {
        exit_error(this, "Mutex 'mutex_deq' not locked!");
    }
    // Dequeue the element
    void* value = Queue_deq((*this).queue);
    // Unlock the mutex_deq mutex and check that it has been done
    if (pthread_mutex_unlock(&(*this).mutex_deq)) {
        exit_error(this, "Mutex 'mutex_deq' not unlocked!");
    }
    // Increment the sem_enq semaphore and check that it has been done
    if (sem_post(&(*this).sem_enq)) {
        exit_error(this, "Semaphore 'sem_enq' not incremented!");
    }
    return value;
}

int BlockingQueue_size(BlockingQueue* this) {
    return Queue_size((*this).queue);
}

bool BlockingQueue_isEmpty(BlockingQueue* this) {
    return Queue_isEmpty((*this).queue);
}

void BlockingQueue_clear(BlockingQueue* this) {
    Queue_clear((*this).queue);
}

void BlockingQueue_destroy(BlockingQueue* this) {
    // Destroy both mutexes
    pthread_mutex_destroy(&(*this).mutex_enq);
    pthread_mutex_destroy(&(*this).mutex_deq);

    // Destroy both semaphores
    sem_destroy(&(*this).sem_enq);
    sem_destroy(&(*this).sem_deq);

    // Destroy the Queue object
    Queue_destroy((*this).queue);
    
    // Free the memory allocated for this blocking queue
    free(this);
}

void exit_error(BlockingQueue* this, char* msg) {
    perror(msg);
    BlockingQueue_destroy(this);
    exit(EXIT_FAILURE);
}
