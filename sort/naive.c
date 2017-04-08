#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

int get_sorted_index(int* list, int list_len, int index);

int main(int argc, char** argv) {
  int my_rank;
  int np;
  int list[] = {2,5,8,5,8,7,3,2,8,11};

  int tag = 0;
  MPI_Status status;
  int list_len = 10;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  if(my_rank == 0){
    int *sorted_list = (int *)malloc(list_len*sizeof(int));
    int sorted_index = get_sorted_index(list, list_len, my_rank);
    sorted_list[sorted_index] = list[my_rank];
    for(int i = 1; i < np; i++){
      int received_sorted_index;
      MPI_Recv(&received_sorted_index, 1, MPI_INT, i, tag, MPI_COMM_WORLD, &status);
      sorted_list[received_sorted_index] = list[i];
    }
    for(int i = 0; i < list_len; i++){
      printf("%d ", sorted_list[i]);
    }
    printf("\n");
    free(sorted_list);
  }else{
    int sorted_index = get_sorted_index(list, list_len, my_rank);
    MPI_Send(&sorted_index, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
  }

  MPI_Finalize();
}

int get_sorted_index(int* list, int list_len, int index) {
  int sorted_index = 0;
  int offset = 0;
  for (int i = 0; i < list_len; i++) {
    if(list[i] < list[index]){
      sorted_index++;
    }
    if(list[i] == list[index] && i < index){
      offset++;
    }
  }
  return sorted_index+offset;
}