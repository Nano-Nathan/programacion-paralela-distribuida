#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void *executeType1 () {
    printf("Type 1.");    
}

void *executeType2 () {
    printf("Type 2.");
}

int main(int count, char *parameters[]) {
    int N, M;
    if(count != 2){
        N = atoi(parameters[0]);
        M = atoi(parameters[0]);
        if(N != 0 && M != 0){
            printf("Ha ingresado N = %d y M = %d", N, M);

            //Array with threads
            pthread_t aThreads[N + M];

            //Create type 1 thread
            for (int i = 0; i < N; i++) {
                pthread_t t;
                if (pthread_create(&t, NULL, executeType1, 0) == 0){
                    printf("No se ha podido crear el thread %d.", i);
                } else {
                    aThreads[i] = t;
                }
            }

            //Create type 2 thread
            for (int i = N; i < N + M; i++) {
                pthread_t t;
                if (pthread_create(&t, NULL, executeType2, 0) == 0){
                    printf("No se ha podido crear el thread %d.", i);
                } else {
                    aThreads[i] = t;
                }
            }
            
            //Wait threads
            for (int i = 0; i < N + M; i++){
                pthread_join(aThreads[i], 0);
            }

            printf("Fin del programa.");
        } else {
            printf("Los números ingresados no son válidos.");
        }
    } else {
        printf("Ingrese al menos 2 números por parámetro.");
    }
}