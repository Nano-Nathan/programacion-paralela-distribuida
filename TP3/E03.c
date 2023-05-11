#include <stdio.h>
#include <math.h>
#include <mpi.h>

#define COUNT_COLUMNS 4
// mpdboot -n 4 -f nodoscluster
// mpicc –o holamundo holamundo.c

int my_id, new_id = 0, id_in_comm, nproc, color, i;
MPI_Comm newComm;

int getColor (int nproc, int id) {
   if((int)(id / (int)(nproc / COUNT_COLUMNS)) % 2 == 0){
      return 1;
   }
   return 0;
   /*
   for (i = 0; i < countRows; i+=2){
      if (i * COUNT_COLUMNS <= id && id < (i + 1) * COUNT_COLUMNS){
         return 1;
      }
   }
   return 0;
   */
}

int getNewID (int nproc, int id) {
   int countRows = (nproc / COUNT_COLUMNS);
   int countRealRows = (countRows / 2) - 1, newID;

   for (i = 0; i < countRows; i+=2){
      if (i * COUNT_COLUMNS <= id && id < (i + 1)*COUNT_COLUMNS){
         //int aux = ((countRealRows - (i + 1)) * 4);
         //int modulo = id % 4;

         //printf("Modulo de proceso %d es %d y se le suma %d\n", id, id % 4, aux);
         //newID = (id % 4) + ((countRealRows - (i + 1)) * 4);
         newID = (COUNT_COLUMNS - (id % COUNT_COLUMNS)) + COUNT_COLUMNS * countRealRows;
         newID--;
         return newID;
      }
      countRealRows--;
   }
   return 0;
}

int main (int argc, char **argv)
{
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &nproc);

   color = getColor(nproc, my_id);
   
   if(color == 1){
      new_id = getNewID(nproc, my_id);
      //printf("Soy %d, mi color es %d y mi nuevo ID sera: %d\n", my_id, color, new_id);
   } else {
      new_id = my_id;
   }

   MPI_Comm_split(MPI_COMM_WORLD, color, new_id, &newComm);

   MPI_Comm_rank(newComm, &id_in_comm);

   if (color == 1) {
      printf("Mi id original es %d y en el nuevo comunicador soy %d.\n", my_id, id_in_comm);
   }

   //printf("Hola Mundo 2023! Soy %d de %d corriendo en %s\n", my_id, nproc, name);
   MPI_Comm_free(&newComm);
   MPI_Finalize();
}