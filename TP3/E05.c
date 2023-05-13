#include <stdio.h>
#include <mpi.h>
// mpdboot -n 4 -f nodoscluster
// mpicc –o holamundo holamundo.c

int Bcast (int source, char *msg, int tam) {
    int my_id, nproc, i;
    MPI_Status status;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
    if (my_id == source){
        MPI_Comm_size(MPI_COMM_WORLD, &nproc);
        MPI_Request requests[nproc];

        //envia el mensaje
        for (i = 0; i < nproc; i++) {
            if (i != source){
                MPI_Isend(&msg, tam, MPI_CHAR, i, 0, MPI_COMM_WORLD, requests[i]);
            }
        }

        printf("Soy %d y he enviado %s a los demas\n", my_id, msg);

        //Espera la recepcion
        for (i = 0; i < nproc; i++){
            if(i != source){
                MPI_Wait(requests[i], &status);
            }
        }
    } else {
        //Recibe el mensaje y lo muestra por pantalla
        MPI_Recv(&msg, tam, MPI_CHAR, source, 0, MPI_COMM_WORLD, &status);
        printf("Soy %d y he recibido %s de %d\n", my_id, msg, source);
    }
}

int main (int argc, char **argv){
    char message[10] = "Mensaje";
    MPI_Init(&argc, &argv);
    Bcast(0, &message, 10);
    MPI_Finalize();
}