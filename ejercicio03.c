#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFF_SIZE   5
#define NP          3
#define NC          3
#define NITERS      4

typedef struct {
    int buf[BUFF_SIZE];
    int in;
    int out;
    sem_t full;
    sem_t empty;
    sem_t mutex;
} sbuf_t;

sbuf_t shared;

void *Producer(void *arg)
{
    int i, item, index;

    index = (int)(long)arg;

    for (i = 0; i < NITERS; i++) {

        /* Produce item */
        item = i;

        /* Prepare to write item to buf */

        /* If there are no empty slots, wait */
        sem_wait(&shared.empty);

        /* If another thread uses the buffer, wait */
        sem_wait(&shared.mutex);

        shared.buf[shared.in] = item;
        shared.in = (shared.in + 1) % BUFF_SIZE;

        printf("[P%d] Producing %d ...\n", index, item);
        fflush(stdout);

        /* Release the buffer */
        sem_post(&shared.mutex);

        /* Increment the number of full slots */
        sem_post(&shared.full);

        /* Interleave producer and consumer execution */
        if (i % 2 == 1)
            sleep(1);
    }

    return NULL;
}

void *Consumer(void *arg)
{
    int i, item, index;

    index = (int)(long)arg;

    for (i = 0; i < NITERS; i++) {

        /* If there are no full slots, wait */
        sem_wait(&shared.full);

        /* If another thread uses the buffer, wait */
        sem_wait(&shared.mutex);

        item = shared.buf[shared.out];
        shared.out = (shared.out + 1) % BUFF_SIZE;

        printf("------> [C%d] consumed %d\n", index, item);
        fflush(stdout);

        /* Release the buffer */
        sem_post(&shared.mutex);

        /* Increment the number of empty slots */
        sem_post(&shared.empty);

        /* Interleave producer and consumer execution */
        if (i % 2 == 1)
            sleep(1);
    }

    return NULL;
}

int main()
{
    pthread_t idP, idC;
    int index;

    sem_init(&shared.full, 0, 0);
    sem_init(&shared.empty, 0, BUFF_SIZE);

    /* Initialize mutex */
    sem_init(&shared.mutex, 0, 1);

    for (index = 0; index < NP; index++)
    {
        /* Create a new producer */
        pthread_create(&idP, NULL, Producer, (void *)(long)index);
    }

    for (index = 0; index < NC; index++)
    {
        /* Create a new consumer */
        pthread_create(&idC, NULL, Consumer, (void *)(long)index);
    }

    pthread_exit(NULL);
}
