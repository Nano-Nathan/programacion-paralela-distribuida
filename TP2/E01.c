#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void *executeType1 () {
    printf("Type 1.\n");
}

void *executeType2 () {
    printf("Type 2.\n");
}

int main(int count, char *parameters[]) {
    int N, M;
    if(count == 3){
        N = atoi(parameters[1]);
        M = atoi(parameters[2]);
        if(N != 0 && M != 0){
            printf("Ha ingresado N = %d y M = %d\n", N, M);

            //Array with threads
            pthread_t aThreads[N + M];

            //Create type 1 thread
            for (int i = 0; i < N; i++) {
                pthread_t t;
                if (pthread_create(&t, NULL, executeType1, NULL) != 0){
                    printf("No se ha podido crear el thread %d.\n", i);
                } else {
                    aThreads[i] = t;
                }
            }

            //Create type 2 thread
            for (int i = N; i < N + M; i++) {
                pthread_t t;
                if (pthread_create(&t, NULL, executeType2, NULL) != 0){
                    printf("No se ha podido crear el thread %d.\n", i);
                } else {
                    aThreads[i] = t;
                }
            }

            //Wait threads
            for (int i = 0; i < N + M; i++){
                pthread_join(aThreads[i], NULL);
            }

            printf("Fin del programa.\n");
        } else {
            printf("Los números ingresados no son válidos.\n");
        }
    } else {
        printf("Ingrese al menos 2 números por parámetro.\n");
    }
}