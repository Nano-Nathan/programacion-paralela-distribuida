#include <stdio.h>
#include <mpi.h>
// mpdboot -n 4 -f nodoscluster
// mpicc –o holamundo holamundo.c

int my_id, id_recv, nproc, resultlen;
char name[30];
MPI_Status status;

int main (int argc, char **argv)
{
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &nproc);
   MPI_Get_processor_name(name, &resultlen);
   name[resultlen] = '\0';
   
   if(nproc > 1){
      if(my_id == 0){
         MPI_Send(&my_id, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
         printf("Soy %d y envie mi id a 1\n", my_id);
         MPI_Recv(&id_recv, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
         printf("Soy %d y recibi %d de 1\n", my_id, id_recv);
      } else {
         MPI_Recv(&id_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
         printf("Soy %d y recibi %d de 0\n", my_id, id_recv);
         MPI_Send(&id_recv, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
         printf("Soy %d y envie %d a 0\n", my_id, id_recv);
      }
   }

   /*MPI_Sendrecv(
      //Send
      &my_id,
      1,
      MPI_INT,
      to_send,
      0, 
      //Recv
      &id_recv,
      1,
      MPI_INT,
      to_recv,
      0,
      //Controllers
      MPI_COMM_WORLD,
      &status
   );*/

   //printf("Soy %d corriendo en %s. He enviado %d a %d y %d ha retornado %d\n", my_id, nproc, name, my_id, to_send, to_recv, id_recv);

   MPI_Finalize();
}