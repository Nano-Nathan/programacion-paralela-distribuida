#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void executeClient (int pWrite [], int pRead []) {
    char message[20];
    //Await message from parent
    read(pRead[0], message, sizeof(message));
    printf("Mensaje recibido del padre: %s\n", message);
    //Close Read extrem
    close(pRead[0]);

    //Send message from parent
    write(pWrite[1], "OK", 2);
    //Close Write extrem
    close(pWrite[1]);

    //Close Read extrem
    //close(pRead[0]);
    //Close Write extrem
    //close(pWrite[1]);
}

void executeServer(int N, int pWrite [N][2], int pRead [N][2]) {
    for (int i = 0; i < N; i++){
        //Send message test
        write(pWrite[i][1], "Esto envia el padre", 20);
        //Close Write extrem
        close(pWrite[i][1]);

        //Await response childen
        char response[2];
        read(pRead[i][0], response, 2);
        printf("Mensaje recibido del proceso hijo %d: %s\n", i, response);
        //Close Read extrem
        close(pRead[i][0]);
    }
    printf("Fin del programa.\n");
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
                executeServer(N, pipesR, pipesW);
            }
        } else {
            printf("El números ingresados no es válido.\n");
        }
    } else {
        printf("Ingrese al menos 1 número por parámetro.\n");
    }
}