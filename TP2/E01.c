#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int count, char *parameters[])
{
    int N, M;
    if(count != 2){
        N = atoi(parameters[0]);
        M = atoi(parameters[0]);
        if(N != 0 || M != 0){
            printf("Ha ingresado N = %d y M = %d", N, M);
        } else {
            printf("Los números ingresados no son válidos.");
        }
    } else {
        printf("Ingrese al menos 2 números por parámetro.");
    }
}