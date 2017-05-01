#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>

/* the number of data elements in each process */
#define N 1000

void init_list(int* list, int list_len) {
  srand(0);
  for (int i = 0; i < list_len; i++) {
    list[i] = rand( ) % 100;
  }
}

void print_list(int* list, int list_len) {
  for(int i = 0; i < list_len; i++){
    printf("%d ", list[i]);
  }
  printf("\n");
}

void naive_sort(int* list, int list_len, int rank, int np, int* sorted_list);
void poet_sort(int* data, int rank, int np, int* sorted_list);
void naive_sort2(int* data, int rank, int np, int* sorted_list);

int main(int argc, char** argv) {
  int rank, np;
  double t1, t2;
  
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &np);

  int list_len = np * N;
  int list[list_len], sorted_list[list_len], local_list[N];

  init_list(list, list_len);

  // if (rank == 0) {
  //   printf("List to sort: ");
  //   print_list(list, list_len);
  // }

  MPI_Barrier(MPI_COMM_WORLD);

  // BEGIN NAIVE

  if (rank == 0) {
    printf("Naive Parallel Sort:\n");
    t1 = MPI_Wtime();
  }

  naive_sort(list, list_len, rank, np, sorted_list);

  if (rank == 0) {
    t2 = MPI_Wtime();
    // printf("Result: ");
    // print_list(sorted_list, list_len);
    printf("Elapsed time is %f\n", t2 - t1);
    printf("\n");
  }

  // END NAIVE

  MPI_Barrier(MPI_COMM_WORLD);

  // BEGIN NAIVE 2

  memcpy(local_list, &list[rank * N], N * sizeof(int));

  if (rank == 0) {
    printf("Naive 2 Parallel Sort:\n");
    t1 = MPI_Wtime();
  }

  naive_sort2(local_list, rank, np, sorted_list);
  
  if (rank == 0) {
    t2 = MPI_Wtime();
    // printf("Result: ");
    // print_list(sorted_list, list_len);
    printf("Elapsed time is %f\n", t2 - t1);
    printf("\n");
  }

  // END NAIVE 2

  MPI_Barrier(MPI_COMM_WORLD);

  // BEGIN POET

  memcpy(local_list, &list[rank * N], N * sizeof(int));

  if (rank == 0) {
    printf("POET Parallel Sort:\n");
    t1 = MPI_Wtime();
  }

  poet_sort(local_list, rank, np, sorted_list);
  
  if (rank == 0) {
    t2 = MPI_Wtime();
    // printf("Result: ");
    // print_list(sorted_list, list_len);
    printf("Elapsed time is %f\n", t2 - t1);
    printf("\n");
  }

  // END POET

  MPI_Finalize();

  return 0;  
}

/* ===============================
              NAIVE
=============================== */

int* get_sorted_indexes(int* list, int list_len, int rank);

void naive_sort(int* list, int list_len, int rank, int np, int* sorted_list) {
  int *sorted_indexes = get_sorted_indexes(list, list_len, rank);

  if(rank == 0) {

    for (int i = 0; i < N; i++) {
      int sorted_index = sorted_indexes[i];
      sorted_list[sorted_index] = list[i];
    }

    for(int p = 1; p < np; p++){
      MPI_Recv(sorted_indexes, N, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      for (int i = 0; i < N; i++) {
        int sorted_index = sorted_indexes[i];
        sorted_list[sorted_index] = list[(p*N)+i];
      }
    }
  }
  else {
    MPI_Send(sorted_indexes, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }

  free(sorted_indexes);
}

int* get_sorted_indexes(int* list, int list_len, int rank) {
  int start = rank * N;
  int *sorted_indexes = (int *)malloc(N * sizeof(int));

  for (int i = 0; i < N; i++) {
    int target_index = start + i;
    int sorted_index = 0;
    for (int j = 0; j < list_len; j++) {
      if(list[j] < list[target_index]) {
        sorted_index++;
      }
      else if(list[j] == list[target_index] && j < target_index) {
        sorted_index++;
      }
    }
    sorted_indexes[i] = sorted_index;
  }

  return sorted_indexes;
}

/* ===============================
              NAIVE 2
=============================== */

int cmp(const void* ap, const void* bp);
void merge(int* l1, int l1_len, int* l2, int l2_len, int* merged_list);

void naive_sort2(int* data, int rank, int np, int* sorted_list) {
  qsort(data, N, sizeof(int), &cmp);

  if(rank == 0) {
    memcpy(sorted_list, data, N * sizeof(int));

    int l2[N];
    for(int p = 1; p < np; p++){
      MPI_Recv(l2, N, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      int merged_list[p * N + N];
      merge(sorted_list, p * N, l2, N, merged_list);
      memcpy(sorted_list, merged_list, (p * N + N) * sizeof(int));
    }
  }
  else {
    MPI_Send(data, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
}

void merge(int* l1, int l1_len, int* l2, int l2_len, int* merged_list) {
  int i = 0, j = 0;
  for (int k = 0; k < (l1_len+l2_len); k++) {
    if (i == l1_len) {
      merged_list[k] = l2[j++];
    }
    else if (j == l2_len) {
      merged_list[k] = l1[i++];
    }
    else if (l1[i] <= l2[j]) {
      merged_list[k] = l1[i++];
    }
    else {
      merged_list[k] = l2[j++];
    }
  }
}

/* ===============================
              POET
=============================== */

/* Adapted from: http://cs.umw.edu/~finlayson/class/fall14/cpsc425/notes/18-sorting.html */

/* comparison function for qsort */
int cmp(const void* ap, const void* bp) {
  int a = * ((const int*) ap);
  int b = * ((const int*) bp);

  if (a < b) {
    return -1;
  } else if (a > b) {
    return 1;
  } else {
    return 0;
  }
}

/* do the parallel odd/even sort */
void parallel_sort(int* data, int rank, int size) {
  int i;

  /* the array we use for reading from partner */
  int other[N];

  /* sort our local array */
  qsort(data, N, sizeof(int), &cmp);

  /* we need to apply P phases where P is the number of processes */
  for (i = 0; i < size; i++) {
    /* find our partner on this phase */
    int partner;

    /* if it's an even phase */
    if (i % 2 == 0) {
      /* if we are an even process */
      if (rank % 2 == 0) {
        partner = rank + 1;
      } else {
        partner = rank - 1;
      }
    } else {
      /* it's an odd phase - do the opposite */
      if (rank % 2 == 0) {
        partner = rank - 1;
      } else {
        partner = rank + 1;
      }
    }

    /* if the partner is invalid, we should simply move on to the next iteration */
    if (partner < 0 || partner >= size) {
      continue;
    }

    /* do the exchange - even processes send first and odd processes receive first
     * this avoids possible deadlock of two processes working together both sending */
    if (rank % 2 == 0) {
      MPI_Send(data, N, MPI_INT, partner, 0, MPI_COMM_WORLD);
      MPI_Recv(other, N, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
      MPI_Recv(other, N, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Send(data, N, MPI_INT, partner, 0, MPI_COMM_WORLD);
    }

    if (rank < partner) {
      int lowest[N];
      int *l1 = data, *l2 = other;
      int i = 0, j = 0;
      for (int k = 0; k < N; k++) {
        if (l1[i] <= l2[j]) {
          lowest[k] = l1[i++];
        }
        else {
          lowest[k] = l2[j++];
        }
      }
      memcpy(data, lowest, N * sizeof(int));
    }
    else {
      int highest[N];
      int *l1 = data, *l2 = other;
      int i = N-1, j = N-1;
      for (int k = 0; k < N; k++) {
        if (l1[i] >= l2[j]) {
          highest[k] = l1[i--];
        }
        else {
          highest[k] = l2[j--];
        }
      }
      memcpy(data, highest, N * sizeof(int));
    }
  }
}

void poet_sort(int* data, int rank, int np, int* sorted_list) {
  parallel_sort(data, rank, np);

  if(rank == 0) {
    memcpy(sorted_list, data, N * sizeof(int));

    for(int p = 1; p < np; p++){
      MPI_Recv(&sorted_list[p*N], N, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }
  else {
    MPI_Send(data, N, MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
}