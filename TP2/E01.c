#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int compartida = 0;
pthread_mutex_t mutex;

void *executeType1 (void *id) {
    //Show message
    printf("Thread1 instancia %d\n", (int *)id);

    //Sleep random time between 0 and 1
    sleep((double)rand() / RAND_MAX);

    //Increase variable
    pthread_mutex_lock(&mutex);
    compartida++;
    pthread_mutex_unlock(&mutex);

    //End
    pthread_exit(NULL);
}

void *executeType2 (void *id) {
    //Show message
    printf("Thread2 instancia %d\n", (int *)id);

    //Sleep random time between 0 and 1
    sleep((double)rand() / RAND_MAX);

    //Show variable
    pthread_mutex_lock(&mutex);
    printf("%d\n", compartida);
    pthread_mutex_unlock(&mutex);

    //End
    pthread_exit(NULL);
}

int main(int count, char *parameters[]) {
    int N, M;
    if(count == 3){
        N = atoi(parameters[1]);
        M = atoi(parameters[2]);
        if(N != 0 && M != 0){
            printf("Ha ingresado N = %d y M = %d\n", N, M);

            //Init mutex
            pthread_mutex_init(&mutex, NULL);

            //Array with threads
            pthread_t aThreads[N + M];

            //Create type 1 thread
            for (int i = 0; i < N; i++) {
                pthread_t t;
                if (pthread_create(&t, NULL, executeType1, (void *)i) != 0){
                    printf("No se ha podido crear el thread %d.\n", i);
                } else {
                    aThreads[i] = t;
                }
            }

            //Create type 2 thread
            for (int i = N; i < N + M; i++) {
                pthread_t t;
                if (pthread_create(&t, NULL, executeType2, (void *)i) != 0){
                    printf("No se ha podido crear el thread %d.\n", i);
                } else {
                    aThreads[i] = t;
                }
            }

            //Wait threads
            for (int i = 0; i < N + M; i++){
                pthread_join(aThreads[i], NULL);
            }

            //Destroy mutex
            pthread_mutex_destroy(&mutex);

            printf("Fin del programa.\n");
        } else {
            printf("Los números ingresados no son válidos.\n");
        }
    } else {
        printf("Ingrese al menos 2 números por parámetro.\n");
    }
}