#include<pthread.h>
#include<stdio.h>
#include<semaphore.h>
#include <stdlib.h>

int n;
int head, tail;
int* buffer;
sem_t psem, csem;
sem_t mut;
void *producer(void *param);
void *consumer(void *param);
int BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS, UPPER, CRITICAL_LENGTH;
int consumed;

struct params {
        int tid;
        int buffer_size;
        int upper_limit;
};

int main(int argc, char **argv) {

        BUFFER_SIZE = atoi(argv[1]);
        NUM_PRODUCERS = atoi(argv[2]);
        NUM_CONSUMERS = atoi(argv[3]);
        UPPER = atoi(argv[4]);
        CRITICAL_LENGTH = 1;//atoi(argv[5]);

        printf("BUFFER SIZE: %d\nNUM PROD: %d\nNUM CONS: %d\nUPPER: %d\n", BUFFER_SIZE, NUM_PRODUCERS, NUM_CONSUMERS, UPPER);

        buffer = malloc(BUFFER_SIZE * sizeof(int));

        consumed = 0;
        n = 0;
        head = 0;
        tail = 0;

        sem_init(&psem, 0, 1);
        sem_init(&csem, 0, 0);
        sem_init(&mut, 0, 1);

        double start = clock();

        pthread_t prod_tids[NUM_PRODUCERS];
        pthread_attr_t prod_attr[NUM_PRODUCERS];
        for (int i = 0; i < NUM_PRODUCERS; i++) {

                struct params *data = (struct params*) malloc(sizeof(struct params));
                data->tid = i;
                data->buffer_size = BUFFER_SIZE;
                data->upper_limit = UPPER;

                pthread_attr_init(&(prod_attr[i]));
                pthread_create(&(prod_tids[i]), &(prod_attr[i]), producer, data);

        }

        pthread_t con_tids[NUM_CONSUMERS];
        pthread_attr_t con_attr[NUM_CONSUMERS];
        for (int i = 0; i < NUM_CONSUMERS; i++) {

                struct params *data = (struct params*) malloc(sizeof(struct params));

                data->tid = i;
                data->buffer_size = BUFFER_SIZE;
                data->upper_limit = UPPER;
                pthread_attr_init(&(con_attr[i]));
                pthread_create(&(con_tids[i]), &(con_attr[i]), consumer, data);

        }

        for (int i = 0; i < NUM_PRODUCERS; i++)
                pthread_join(prod_tids[i], NULL);

        for (int i = 0; i < NUM_CONSUMERS; i++)
                pthread_join(con_tids[i], NULL);

        double end = clock();
        double elapse_time = ((double) (end - start))/ CLOCKS_PER_SEC;
        printf("main thread elapse_time %f\n", elapse_time);

}

void show() {
        printf("------------\nHead: %d\nTail: %d\n", buffer[head], buffer[tail]);
        for (int i = 0; i < BUFFER_SIZE; i++) {
                printf("%d\n", buffer[i]);
        }
}

void *producer(void *param) {

        struct params *data;
        data = (struct params*) param;

        //printf("Producer: %d starts\n", data->tid);

        while (n < data->upper_limit) {
                sem_wait(&psem);
                sem_wait(&mut);

                if (n >= data->upper_limit) {
                        sem_post(&mut);
                        sem_post(&psem);
                        pthread_exit(0);
                }

                //printf("Producing %d\n", n);
                for(int j = 0; j < CRITICAL_LENGTH; j++) {
                        buffer[tail] = n++;
                        tail = (tail + 1) % data->buffer_size;
                }
                sem_post(&mut);
                sem_post(&csem);
                if (n >= data->upper_limit) {
                        sem_post(&psem);
                        pthread_exit(0);
                }

           }

        //printf("Producer: %d exits\n", data->tid);
        pthread_exit(0);
}

void *consumer(void *param) {

        struct params *data;
        data = (struct params*) param;

        //printf("Consumer: %d starts\n", data->tid);
        while (consumed < data->upper_limit) {

                //printf("HERE%d\n", data->tid);
                sem_wait(&csem);
                sem_wait(&mut);

                if (consumed >= data->upper_limit) {
                        sem_post(&mut);
                        sem_post(&csem);
                        pthread_exit(0);
                }
                //printf("HERE%d\n", data->tid);
                for (int j = 0; j < CRITICAL_LENGTH; j++) {
                        printf("<%d><%d>\n", buffer[head], data->tid);
                        head = (head + 1) % data->buffer_size;
                        consumed++;
                }

                //printf("HERE%d\n", data->tid);
                sem_post(&psem);
                sem_post(&mut);
                if (n >= data->upper_limit) {
                        sem_post(&csem);
                        pthread_exit(0);
                }
                //printf("HERE%d\n", data->tid);
        }


                //printf("HERE%d\n", data->tid);
        pthread_exit(0);
}
