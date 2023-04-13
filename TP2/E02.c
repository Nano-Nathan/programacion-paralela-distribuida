#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int compartida = 0;

void executeType1 () {
    printf("Type 1.\n");
}

void executeType2 () {
    printf("Type 2.\n");
}

void executeParent() {
    printf("Fin del programa.\n");
}

int main(int count, char *parameters[]) {
    int N, M;
    if(count == 3){
        N = atoi(parameters[1]);
        M = atoi(parameters[2]);
        if(N != 0 && M != 0){
            printf("Ha ingresado N = %d y M = %d\n", N, M);

            //id children process
            int idC;

            //Create type 1 process
            for (int i = 0; i < N; i++) {
                idC = fork();
                if(idC == 0){
                    executeType1();
                    break;
                }
            }

            if(idC != 0){
                //Create type 2 process
                for (int i = N; i < N + M; i++) {
                    idC = fork();
                    if(idC == 0){
                        executeType2();
                        break;
                    }
                }

                //End program
                if(idC != 0){
                    executeParent();
                }
            }
        } else {
            printf("Los números ingresados no son válidos.\n");
        }
    } else {
        printf("Ingrese al menos 2 números por parámetro.\n");
    }
}