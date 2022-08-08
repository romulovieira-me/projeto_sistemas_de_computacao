#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include </usr/include/semaphore.h>

#define BUFF_SIZE   5		/* total number of slots */
#define NP          3		/* total number of producers */
#define NCP	    3		/* total number of producers */
#define NC          3		/* total number of consumers */
#define NITERS      4		/* number of items produced/consumed */

typedef struct {
    int buf[BUFF_SIZE];   /* shared var */
    int in;         	  /* buf[in%BUFF_SIZE] is the first empty slot */
    int out;        	  /* buf[out%BUFF_SIZE] is the first full slot */
    sem_t full;     	  /* keep track of the number of full spots */
    sem_t empty;    	  /* keep track of the number of empty spots */
    sem_t mutex;    	  /* enforce mutual exclusion to shared data */
} sbuf_t;

sbuf_t shared0;
sbuf_t shared1;

void *Producer(void *arg)
{
    int i, item, index;

    index = (int)arg;

    for (i=0; i < NITERS; i++) {

        /* Produce item */
        item = i;	

        /* Prepare to write item to buf */

        /* If there are no empty slots, wait */
        sem_wait(&shared0.empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared0.mutex);
        shared0.buf[shared0.in] = 1000*index + item;
        shared0.in = (shared0.in+1)%BUFF_SIZE;
        printf("[P%d] Producing %d ...\n", index, 1000*index + item); fflush(stdout);
        /* Release the buffer */
        sem_post(&shared0.mutex);
        /* Increment the number of full slots */
        sem_post(&shared0.full);

    }
    return NULL;
}

void *ConsumerProducer(void *arg)
{
    int i, item, index;

    index = (int)arg;

    for (i=0; i < NITERS; i++) {


	/* If there are no filled slots, wait */
        sem_wait(&shared0.full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared0.mutex);
        item = shared0.buf[shared0.out];
        shared0.out = (shared0.out+1)%BUFF_SIZE;
        printf("[CP%d] Consuming %d ...\n", index, item); fflush(stdout);
        /* Release the buffer */
        sem_post(&shared0.mutex);
        /* Increment the number of empty slots */
        sem_post(&shared0.empty);
     	

        /* Prepare to write item to buf */

        /* If there are no empty slots, wait */
        sem_wait(&shared1.empty);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared1.mutex);
        shared1.buf[shared1.in] = item;
        shared1.in = (shared1.in+1)%BUFF_SIZE;
        printf("[CP%d] Producing %d ...\n", index, item); fflush(stdout);
        /* Release the buffer */
        sem_post(&shared1.mutex);
        /* Increment the number of full slots */
        sem_post(&shared1.full);

    }
    return NULL;
}


void *Consumer(void *arg)
{
    int i, item, index;

    index = (int)arg;

    for (i=0; i < NITERS; i++) {

        /* Prepare to read item from buf */

        /* If there are no filled slots, wait */
        sem_wait(&shared1.full);
        /* If another thread uses the buffer, wait */
        sem_wait(&shared1.mutex);
        item = shared1.buf[shared1.out];
        shared1.out = (shared1.out+1)%BUFF_SIZE;
        printf("[C%d] Consuming %d ...\n", index, item); fflush(stdout);
        /* Release the buffer */
        sem_post(&shared1.mutex);
        /* Increment the number of empty slots */
        sem_post(&shared1.empty);

    }
    return NULL;
}

int main()
{
    pthread_t idP, idC, idCP;
    int index;
    int sP, sC, sCP;

    sem_init(&shared0.full, 0, 0);
    sem_init(&shared0.empty, 0, BUFF_SIZE);
    sem_init(&shared0.mutex, 0, 1);
    sem_init(&shared1.full, 0, 0);
    sem_init(&shared1.empty, 0, BUFF_SIZE);
    sem_init(&shared1.mutex, 0, 1);

    for (index = 0; index < NP; index++)
    {  
       /* Create a new producer */
       pthread_create(&idP, NULL, Producer, (void*)index);
    }

    for (index = 0; index < NCP; index++)
    {  
       /* Create a new producer */
       pthread_create(&idCP, NULL, ConsumerProducer, (void*)index);
    }

    for (index = 0; index < NC; index++)
    {  
       /* Create a new consumer */
       pthread_create(&idC, NULL, Consumer, (void*)index);
    }

    pthread_exit(NULL);
}
