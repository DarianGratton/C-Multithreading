#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define NPRODUCERS 5
#define NCONSUMERS 5
#define NITEMS 100
#define MAX 1000

// int count = 0;

pthread_mutex_t mutex;
pthread_cond_t condc;
pthread_cond_t condp;
int buffer;

struct {
  int nused;        /* number of items currently in the buffer */
  int next_write;   /* next location to store data (by producer) */
  int next_read;    /* next location to retrieve data (by consumer) */
  int data[NITEMS]; /* actual data (the buffer) */
} buf = {0, 0, 0, {0}};

void * consumer(void * thread_void_ptr) {
  int j;
  for (j = 1; j <= MAX; ++j) {
    long id = (long) thread_void_ptr;
    int i;

    pthread_mutex_lock(&mutex);
    while(buffer == 0) {
      pthread_cond_wait(&condc, &mutex);
    }

    buffer = 0;
    i = buf.next_read;
    printf("C%ld: %d <-- [%d]\n", id, buf.data[i], i); 
    buf.next_read = (buf.next_read + 1) % NITEMS;
    
    pthread_cond_signal(&condp);
    pthread_mutex_unlock(&mutex);
  }
  
  pthread_exit(0);
  return NULL;
}

void * producer(void * thread_void_ptr) {
  int j;
  for(j = 1; j <= MAX; ++j) {
    long id = (long) thread_void_ptr;
    pthread_mutex_lock(&mutex);
    
    while (buffer != 0) {
      pthread_cond_wait(&condp, &mutex);
    }

    buffer = 1;
    int i = buf.next_write;
    buf.data[buf.next_write] = i;
    printf("P%ld: %d --> [%d]\n", id, buf.data[i], i);
    buf.next_write = (buf.next_write + 1) % NITEMS;

    pthread_cond_signal(&condc);
    pthread_mutex_unlock(&mutex); 
  }
  pthread_exit(0);
  return NULL;
}

int main(void) {
  pthread_t tid1[NPRODUCERS];
  pthread_t tid2[NCONSUMERS];
  long i;

  /* Initializing semaphores */
  pthread_mutex_init(&mutex, 0);
  pthread_cond_init(&condc, 0);
  pthread_cond_init(&condp, 0); 

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

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condc);
  pthread_cond_destroy(&condp);

  return 0;
}
