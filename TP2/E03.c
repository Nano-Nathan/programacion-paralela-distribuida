#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// -1: Abandona, 0: Se planta, 1: Continua
void executeClient (int pWrite [], int pRead []) {
    int decision = 1;
    double score = 0;
    double currentCart = 1;

    //Mientras el juego no haya terminado y siga jugando
    while (currentCart > 0 && decision > 0){
        //Solicita la carta al padre
        read(pRead[0], &currentCart, sizeof(double));
        //Valida si no se ha terminado el juego
        if(currentCart > 0){
            //Aumenta su score
            score += currentCart;

            //Toma la decision
            if(score > 5.5){
                //Abandona o se planta
                if(score > 7.5){
                    decision = -1;
                } else {
                    decision = 0;
                }
            }

            //Avisa al padre si continua o no
            write(pWrite[1], &decision, sizeof(int));
        }
    }

    //Al terminar la ejecucion, envia el score y cierra las conexiones
    write(pWrite[1], &score, sizeof(double));
    close(pRead[0]);
    close(pWrite[1]);
}


void shuffle(double *array, int n) {
    srand(time(NULL));
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        double temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// -1: Abandona, 0: Se planta, 1: Continua
void executeServer(int N, int pWrite [N][2], int pRead [N][2], int clients[N]) {
    int status[N];
    int countP = 0;
    int countA = 0;
    int currentCart = 0;
    int size = 40;
    double scores[N];
    double carts[] = {
        1, 2, 3, 4, 5, 6, 7, 0.5, 0.5, 0.5,
        1, 2, 3, 4, 5, 6, 7, 0.5, 0.5, 0.5,
        1, 2, 3, 4, 5, 6, 7, 0.5, 0.5, 0.5,
        1, 2, 3, 4, 5, 6, 7, 0.5, 0.5, 0.5
    };

    //Inicializo los estados y scores
    for (int i = 0; i < N; i++){
        status[i] = 1;
        scores[i] = 0;
    }
    
    //Mezclo las cartas
    shuffle(carts, size);

    //Mientras haya jugadores y cartas
    while ((countP + countA < N) && (size - currentCart > N)) {
        //Reparte las cartas
        for (int i = 0; i < N; i++){
            //Si sigue en juego
            if(status[i] == 1){
                //Envia el valor de la carta que le toca
                write(pWrite[i][1], &carts[currentCart], sizeof(double));
                //Selecciona la siguiente carta para repartir
                currentCart++;
            }
        }

        //Espera la decision de cada cliente
        for (int i = 0; i < N; i++){
            //Si continuaba
            if(status[i] == 1){
                //Actualiza el estado del cliente
                int response;
                read(pRead[i][0], &response, sizeof(int));
                status[i] = response;
                //Actualiza los contadores
                if(response < 1){
                    if (response == -1){
                        countA++;
                    } else if (response == 0){
                        countP++;
                    }
                    //Si abandona, consulta los puntos y cierra la conexion
                    double score;
                    read(pRead[i][0], &score, sizeof(double));
                    scores[i] = score;
                    close(pRead[i][0]);
                    close(pWrite[i][1]);
                }
            }
        }
    }

    double flag = 0.0;
    //Una vez terminado el juego, avisa a quienes siguen que termina, espera su score y cierra la conexion
    for (int i = 0; i < N; i++){
        //Si seguia en juego
        if(status[i] == 1){
            //Avisa que termino
            write(pWrite[i][1], &flag, sizeof(double));
            //Espera el score
            double score;
            read(pRead[i][0], &score, sizeof(double));
            scores[i] = score;
            //Cierra las conexiones
            close(pRead[i][0]);
            close(pWrite[i][1]);
        }
    }

    //Elije el ganador
    for (int i = 0; i < N; i++){
        printf("El proceso %d tiene %.1f puntos.\n", i, scores[i]);
    }
}

int main(int count, char *parameters[]) {
    int N;
    if(count == 2){
        N = atoi(parameters[1]);
        if(N != 0){
            printf("Ha ingresado N = %d\n", N);

            //id children process
            int idC;

            //Create array for the pipes to children can write
            int pipesW[N][2];
            //Create array for the pipes to children can read
            int pipesR[N][2];

            //Create array for the children process
            int process[N];

            //Create client process
            for (int i = 0; i < N; i++) {

                //Create pipe for Write
                if (pipe(pipesW[i]) == -1) {
                    printf("Error al crear un pipe.\n");
                }
                //Create pipe for Read
                if (pipe(pipesR[i]) == -1) {
                    printf("Error al crear un pipe.\n");
                }

                //Create process
                idC = fork();
                if(idC == -1){
                    printf("Error al crear un proceso.\n");
                } else if(idC == 0){
                    //Close read extrem
                    close(pipesW[i][0]);
                    //Close write extrem
                    close(pipesR[i][1]);
                    executeClient(pipesW[i], pipesR[i]);
                    break;
                }

                //Save PID
                process[i] = idC;

                //Close write extrem
                close(pipesW[i][1]);
                //Close read extrem
                close(pipesR[i][0]);
            }

            if(idC != 0){
                //Init game
                executeServer(N, pipesR, pipesW, process);
            }
        } else {
            printf("El números ingresados no es válido.\n");
        }
    } else {
        printf("Ingrese al menos 1 número por parámetro.\n");
    }
}