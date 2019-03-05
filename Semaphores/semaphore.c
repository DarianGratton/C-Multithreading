#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NPRODUCERS 2
#define NCONSUMERS 2
#define NITEMS 100

sem_t mutex;
sem_t empty;
sem_t full; 
int count = 0;

struct {
  int nused;        /* number of items currently in the buffer */
  int next_write;   /* next location to store data (by producer) */
  int next_read;    /* next location to retrieve data (by consumer) */
  int data[NITEMS]; /* actual data (the buffer) */
} buf = {0, 0, 0, {0}};

void * consumer(void * thread_void_ptr) {
  while(1) {
    long id = (long) thread_void_ptr;
    int i;
    sem_wait(&full);
    sem_wait(&mutex);
    i = buf.next_read;
    printf("C%ld: %d <-- [%d]\n", id, buf.data[i], i); 
    buf.next_read = (buf.next_read + 1) % NITEMS;
    sem_post(&mutex);
    sem_post(&empty);
  }
  return NULL;
}

void * producer(void * thread_void_ptr) {
  while(1) {
    long id = (long) thread_void_ptr;
    sem_wait(&empty);
    sem_wait(&mutex);
    int i = buf.next_write;
    int item = count++;
    buf.data[buf.next_write] = item;
    printf("P%ld: %d --> [%d]\n", id, buf.data[i], i);
    buf.next_write = (buf.next_write + 1) % NITEMS;
    sem_post(&mutex);
    sem_post(&full); 
  }
  return NULL;
}

int main(void) {
  pthread_t tid1[NPRODUCERS];
  pthread_t tid2[NCONSUMERS];
  long i;

  /* Initializing semaphores */
  sem_init(&mutex, 0, 1);
  sem_init(&full, 0, 0);
  sem_init(&empty, 0, NITEMS); 

  /* Creating producer threads */
  for (i = 0; i < NPRODUCERS; ++i) {
    if (pthread_create(&tid1[i], 0, producer, (void *) i)) {
      perror("pthread_create");
      return 1;
    }
  }
 
  /* Creating consumer threads */
  for (i = 0; i < NCONSUMERS; ++i) {
    if (pthread_create(&tid2[i], 0, consumer, (void *) i)) {
      perror("pthread_create");
      return 3;
    }
  }
  
  
  for (i = 0; i < NPRODUCERS; ++i) {
    if (pthread_join(tid1[i], NULL)) {
      perror("pthread_join");
      return 2;
    }
  }
  
  for (i = 0; i < NPRODUCERS; ++i) {
    if (pthread_join(tid1[i], NULL)) {
      perror("pthread_join");
      return 2;
    }
  }

  for (i = 0; i < NCONSUMERS; ++i) {
    if (pthread_join(tid2[i], NULL)) {
      perror("pthread_join");
      return 4;
    } 
  }

  sleep(4);
  sem_destroy(&mutex);
  sem_destroy(&full);
  sem_destroy(&empty);

  return 0;
}
