#include <stdio.h>
#include <math.h>
#include <mpi.h>

#define COUNT_COLUMNS 4
// mpdboot -n 4 -f nodoscluster
// mpicc –o holamundo holamundo.c

int my_id, new_id = 0, id_in_comm, nproc, color, i;
MPI_Comm newComm;

int getColor (int nproc, int id) {
   if ((int)(id / (int)(nproc / COUNT_COLUMNS)) % 2 == 0){
      return (id % 2 == 0) ? 1 : 0;
   } else {
      return (id % 2 != 0) ? 1 : 0;
   }
}

int getNewID (int nproc, int id) {
   int countRows = (nproc / COUNT_COLUMNS),
      myRow = id / countRows,
      elementsInRow = COUNT_COLUMNS / 2,
      elementsUnder = 0;
   if(myRow % 2 == 0 && COUNT_COLUMNS % 2 != 0){
      elementsInRow + 1;
   }

   for (i = myRow + 1; i < countRows; i++){
      if (COUNT_COLUMNS % 2 != 0){
         if(i % 2 == 0){
            elementsUnder += elementsInRow;
         } else {
            elementsUnder += elementsInRow - 1;
         }
      } else {
         elementsUnder += elementsInRow;
      }
   }
   //printf("My id %d, elementos debajo %d, elementos en la fila %d, mi posición allí %d\n", id, elementsUnder, elementsInRow, elementsInRow - (((COUNT_COLUMNS - (id % COUNT_COLUMNS)) / elementsInRow) % elementsInRow));
   return elementsUnder + elementsInRow - (((COUNT_COLUMNS - (id % COUNT_COLUMNS)) / elementsInRow) % elementsInRow);
}

int main (int argc, char **argv)
{
   MPI_Init(&argc, &argv);
   MPI_Comm_rank(MPI_COMM_WORLD, &my_id);
   MPI_Comm_size(MPI_COMM_WORLD, &nproc);

   color = getColor(nproc, my_id);
   if(color == 1){
      new_id = getNewID(nproc, my_id);
      printf("Soy %d, mi color es %d y mi nuevo id sera %d\n", my_id, color, new_id);
   }
   
   //if(color == 1){
   //   //new_id = getNewID(nproc, my_id);
   //} else {
   //   new_id = my_id;
   //}

   //MPI_Comm_split(MPI_COMM_WORLD, color, new_id, &newComm);

   //MPI_Comm_rank(newComm, &id_in_comm);

   //if (color == 1) {
   //   printf("Mi id original es %d y en el nuevo comunicador soy %d.\n", my_id, id_in_comm);
   //}

   //printf("Hola Mundo 2023! Soy %d de %d corriendo en %s\n", my_id, nproc, name);
   //MPI_Comm_free(&newComm);
   MPI_Finalize();
}