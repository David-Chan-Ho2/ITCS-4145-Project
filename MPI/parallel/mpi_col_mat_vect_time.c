/* File:     mpi_col_mat_vect_time.c
 *
 * Purpose:  Parallel matrix-vector multiplication using a block-column
 *           distribution of the matrix.  The square matrix of order n
 *           is read by process 0 and distributed via a loop of sends.
 *           Each process computes a partial dot product over its columns
 *           for every row, then MPI_Reduce_scatter sums the partial
 *           results and distributes them so that each process holds
 *           local_n consecutive elements of y.
 *
 * Compile:  mpicc -g -Wall -o mpi_col_mat_vect_time mpi_col_mat_vect_time.c
 * Run:      mpiexec -n <comm_sz> ./mpi_col_mat_vect_time <mat_file> <vec_file>
 *
 * Input:    mat_file: text file whose first value is n (the matrix order)
 *                     followed by n*n doubles in row-major order
 *           vec_file: text file containing n doubles for vector x
 * Output:   Elapsed time for the multiplication
 *
 * Notes:
 *    1. n must be positive and evenly divisible by comm_sz
 *    2. Define DEBUG for verbose output including the product vector y
 */
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void Check_for_error(int local_ok, char fname[], char message[],
      MPI_Comm comm);
void Get_dim(int* n_p, int* local_n_p, char mat_file[], int my_rank,
      int comm_sz, MPI_Comm comm);
void Allocate_arrays(double** local_A_pp, double** local_x_pp,
      double** local_y_pp, int n, int local_n, MPI_Comm comm);
void Read_matrix(double local_A[], int n, int local_n, char mat_file[],
      int my_rank, int comm_sz, MPI_Comm comm);
void Read_vector(double local_x[], int n, int local_n, char vec_file[],
      int my_rank, MPI_Comm comm);
void Print_matrix(char title[], double local_A[], int n, int local_n,
      int my_rank, int comm_sz, MPI_Comm comm);
void Print_vector(char title[], double local_vec[], int n, int local_n,
      int my_rank, MPI_Comm comm);
void Mat_vect_mult(double local_A[], double local_x[], double local_y[],
      int n, int local_n, MPI_Comm comm);

/*-------------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   double* local_A;
   double* local_x;
   double* local_y;
   int n, local_n;
   int my_rank, comm_sz;
   MPI_Comm comm;
   double start, finish, loc_elapsed, elapsed;

   MPI_Init(NULL, NULL);
   comm = MPI_COMM_WORLD;
   MPI_Comm_size(comm, &comm_sz);
   MPI_Comm_rank(comm, &my_rank);

   if (argc != 3) {
      if (my_rank == 0)
         fprintf(stderr, "Usage: %s <mat_file> <vec_file>\n", argv[0]);
      MPI_Finalize();
      return 1;
   }

   Get_dim(&n, &local_n, argv[1], my_rank, comm_sz, comm);
   Allocate_arrays(&local_A, &local_x, &local_y, n, local_n, comm);

   Read_matrix(local_A, n, local_n, argv[1], my_rank, comm_sz, comm);
#  ifdef DEBUG
   Print_matrix("A", local_A, n, local_n, my_rank, comm_sz, comm);
#  endif

   Read_vector(local_x, n, local_n, argv[2], my_rank, comm);
#  ifdef DEBUG
   Print_vector("x", local_x, n, local_n, my_rank, comm);
#  endif

   MPI_Barrier(comm);
   start = MPI_Wtime();
   Mat_vect_mult(local_A, local_x, local_y, n, local_n, comm);
   finish = MPI_Wtime();
   loc_elapsed = finish - start;
   MPI_Reduce(&loc_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

#  ifdef DEBUG
   Print_vector("y", local_y, n, local_n, my_rank, comm);
#  endif

   if (my_rank == 0)
      printf("Elapsed time = %e seconds\n", elapsed);

   free(local_A);
   free(local_x);
   free(local_y);
   MPI_Finalize();
   return 0;
}  /* main */


/*-------------------------------------------------------------------*/
void Check_for_error(
      int       local_ok  /* in */,
      char      fname[]   /* in */,
      char      message[] /* in */,
      MPI_Comm  comm      /* in */) {
   int ok;

   MPI_Allreduce(&local_ok, &ok, 1, MPI_INT, MPI_MIN, comm);
   if (ok == 0) {
      int my_rank;
      MPI_Comm_rank(comm, &my_rank);
      if (my_rank == 0) {
         fprintf(stderr, "Proc %d > In %s, %s\n", my_rank, fname, message);
         fflush(stderr);
      }
      MPI_Finalize();
      exit(-1);
   }
}  /* Check_for_error */


/*-------------------------------------------------------------------*/
/* Process 0 opens mat_file, reads the leading integer n, and        *
 * broadcasts it to all processes.                                   */
void Get_dim(
      int*      n_p        /* out */,
      int*      local_n_p  /* out */,
      char      mat_file[] /* in  */,
      int       my_rank    /* in  */,
      int       comm_sz    /* in  */,
      MPI_Comm  comm       /* in  */) {
   int local_ok = 1;

   if (my_rank == 0) {
      int tmp;
      FILE* f = fopen(mat_file, "r");
      /* File header is "n n" (same format as the row-block program) */
      if (f == NULL || fscanf(f, "%d %d", n_p, &tmp) != 2) local_ok = 0;
      if (f != NULL) fclose(f);
   }
   Check_for_error(local_ok, "Get_dim", "Can't read n from matrix file", comm);
   MPI_Bcast(n_p, 1, MPI_INT, 0, comm);
   if (*n_p <= 0 || *n_p % comm_sz != 0) local_ok = 0;
   Check_for_error(local_ok, "Get_dim",
         "n must be positive and evenly divisible by comm_sz", comm);

   *local_n_p = *n_p / comm_sz;
}  /* Get_dim */


/*-------------------------------------------------------------------*/
/* local_A: n rows x local_n cols  (all rows, a block of columns)   */
/* local_x: local_n elements       (the matching block of x)        */
/* local_y: local_n elements       (output block of y)              */
void Allocate_arrays(
      double**  local_A_pp  /* out */,
      double**  local_x_pp  /* out */,
      double**  local_y_pp  /* out */,
      int       n           /* in  */,
      int       local_n     /* in  */,
      MPI_Comm  comm        /* in  */) {
   int local_ok = 1;

   *local_A_pp = malloc(n * local_n * sizeof(double));
   *local_x_pp = malloc(local_n * sizeof(double));
   *local_y_pp = malloc(local_n * sizeof(double));
   if (*local_A_pp == NULL || *local_x_pp == NULL || *local_y_pp == NULL)
      local_ok = 0;
   Check_for_error(local_ok, "Allocate_arrays",
         "Can't allocate local arrays", comm);
}  /* Allocate_arrays */


/*-------------------------------------------------------------------*/
/* Process 0 reads the full n x n matrix from mat_file (skipping the *
 * leading n header), packs each process's column block into a buffer *
 * and sends it.  local_A[i*local_n + j] = A[i][rank*local_n + j]   */
void Read_matrix(
      double    local_A[]  /* out */,
      int       n          /* in  */,
      int       local_n    /* in  */,
      char      mat_file[] /* in  */,
      int       my_rank    /* in  */,
      int       comm_sz    /* in  */,
      MPI_Comm  comm       /* in  */) {
   double* A = NULL;
   double* col_buf = NULL;
   int i, j, p, tmp_m, tmp_n;
   int local_ok = 1;

   if (my_rank == 0) {
      A = malloc(n * n * sizeof(double));
      col_buf = malloc(n * local_n * sizeof(double));
      if (A == NULL || col_buf == NULL) local_ok = 0;
      Check_for_error(local_ok, "Read_matrix",
            "Can't allocate temporary matrix", comm);

      FILE* f = fopen(mat_file, "r");
      if (f == NULL) local_ok = 0;
      Check_for_error(local_ok, "Read_matrix", "Can't open matrix file", comm);
      fscanf(f, "%d %d", &tmp_m, &tmp_n);   /* skip "n n" header */
      for (i = 0; i < n; i++)
         for (j = 0; j < n; j++)
            fscanf(f, "%lf", &A[i*n + j]);
      fclose(f);

      /* Copy process 0's column block directly */
      for (i = 0; i < n; i++)
         for (j = 0; j < local_n; j++)
            local_A[i*local_n + j] = A[i*n + j];

      /* Pack and send each other process its column block */
      for (p = 1; p < comm_sz; p++) {
         for (i = 0; i < n; i++)
            for (j = 0; j < local_n; j++)
               col_buf[i*local_n + j] = A[i*n + p*local_n + j];
         MPI_Send(col_buf, n*local_n, MPI_DOUBLE, p, 0, comm);
      }
      free(A);
      free(col_buf);
   } else {
      Check_for_error(local_ok, "Read_matrix",
            "Can't allocate temporary matrix", comm);
      Check_for_error(local_ok, "Read_matrix", "Can't open matrix file", comm);
      MPI_Recv(local_A, n*local_n, MPI_DOUBLE, 0, 0, comm, MPI_STATUS_IGNORE);
   }
}  /* Read_matrix */


/*-------------------------------------------------------------------*/
/* Process 0 reads n doubles from vec_file and scatters local_n      *
 * elements to each process.                                         */
void Read_vector(
      double    local_x[]  /* out */,
      int       n          /* in  */,
      int       local_n    /* in  */,
      char      vec_file[] /* in  */,
      int       my_rank    /* in  */,
      MPI_Comm  comm       /* in  */) {
   double* x = NULL;
   int i, local_ok = 1;

   if (my_rank == 0) {
      x = malloc(n * sizeof(double));
      if (x == NULL) local_ok = 0;
      Check_for_error(local_ok, "Read_vector",
            "Can't allocate temporary vector", comm);

      FILE* f = fopen(vec_file, "r");
      if (f == NULL) local_ok = 0;
      Check_for_error(local_ok, "Read_vector", "Can't open vector file", comm);
      for (i = 0; i < n; i++)
         fscanf(f, "%lf", &x[i]);
      fclose(f);

      MPI_Scatter(x, local_n, MPI_DOUBLE,
            local_x, local_n, MPI_DOUBLE, 0, comm);
      free(x);
   } else {
      Check_for_error(local_ok, "Read_vector",
            "Can't allocate temporary vector", comm);
      Check_for_error(local_ok, "Read_vector", "Can't open vector file", comm);
      MPI_Scatter(x, local_n, MPI_DOUBLE,
            local_x, local_n, MPI_DOUBLE, 0, comm);
   }
}  /* Read_vector */


/*-------------------------------------------------------------------*/
/* Gather column blocks back to process 0 and print the full matrix. */
void Print_matrix(
      char      title[]   /* in */,
      double    local_A[] /* in */,
      int       n         /* in */,
      int       local_n   /* in */,
      int       my_rank   /* in */,
      int       comm_sz   /* in */,
      MPI_Comm  comm      /* in */) {
   double* A = NULL;
   double* col_buf = NULL;
   int i, j, p, local_ok = 1;

   if (my_rank == 0) {
      A = malloc(n * n * sizeof(double));
      col_buf = malloc(n * local_n * sizeof(double));
      if (A == NULL || col_buf == NULL) local_ok = 0;
      Check_for_error(local_ok, "Print_matrix",
            "Can't allocate temporary matrix", comm);

      for (i = 0; i < n; i++)
         for (j = 0; j < local_n; j++)
            A[i*n + j] = local_A[i*local_n + j];

      for (p = 1; p < comm_sz; p++) {
         MPI_Recv(col_buf, n*local_n, MPI_DOUBLE, p, 0, comm, MPI_STATUS_IGNORE);
         for (i = 0; i < n; i++)
            for (j = 0; j < local_n; j++)
               A[i*n + p*local_n + j] = col_buf[i*local_n + j];
      }

      printf("\nThe matrix %s\n", title);
      for (i = 0; i < n; i++) {
         for (j = 0; j < n; j++)
            printf("%6.3f ", A[i*n + j]);
         printf("\n");
      }
      printf("\n");
      free(A);
      free(col_buf);
   } else {
      Check_for_error(local_ok, "Print_matrix",
            "Can't allocate temporary matrix", comm);
      MPI_Send(local_A, n*local_n, MPI_DOUBLE, 0, 0, comm);
   }
}  /* Print_matrix */


/*-------------------------------------------------------------------*/
/* y and x share the same block distribution, so MPI_Gather works    *
 * identically for both.                                             */
void Print_vector(
      char      title[]     /* in */,
      double    local_vec[] /* in */,
      int       n           /* in */,
      int       local_n     /* in */,
      int       my_rank     /* in */,
      MPI_Comm  comm        /* in */) {
   double* vec = NULL;
   int i, local_ok = 1;

   if (my_rank == 0) {
      vec = malloc(n * sizeof(double));
      if (vec == NULL) local_ok = 0;
      Check_for_error(local_ok, "Print_vector",
            "Can't allocate temporary vector", comm);
      MPI_Gather(local_vec, local_n, MPI_DOUBLE,
            vec, local_n, MPI_DOUBLE, 0, comm);
      printf("\nThe vector %s\n", title);
      for (i = 0; i < n; i++)
         printf("%6.3f ", vec[i]);
      printf("\n");
      free(vec);
   } else {
      Check_for_error(local_ok, "Print_vector",
            "Can't allocate temporary vector", comm);
      MPI_Gather(local_vec, local_n, MPI_DOUBLE,
            vec, local_n, MPI_DOUBLE, 0, comm);
   }
}  /* Print_vector */


/*-------------------------------------------------------------------*/
/* Column-block multiply:                                             *
 *   partial_y[i] = sum_{j=0}^{local_n-1} local_A[i*local_n+j]      *
 *                  * local_x[j]    for all rows i                   *
 *   MPI_Reduce_scatter sums partial_y across processes and gives    *
 *   process p elements [p*local_n, (p+1)*local_n) of the full y.   */
void Mat_vect_mult(
      double    local_A[]  /* in  */,
      double    local_x[]  /* in  */,
      double    local_y[]  /* out */,
      int       n          /* in  */,
      int       local_n    /* in  */,
      MPI_Comm  comm       /* in  */) {
   double* partial_y;
   int* recv_counts;
   int i, j, comm_sz;
   int local_ok = 1;

   MPI_Comm_size(comm, &comm_sz);

   partial_y   = malloc(n * sizeof(double));
   recv_counts = malloc(comm_sz * sizeof(int));
   if (partial_y == NULL || recv_counts == NULL) local_ok = 0;
   Check_for_error(local_ok, "Mat_vect_mult",
         "Can't allocate temporary arrays", comm);

   for (i = 0; i < n; i++) {
      partial_y[i] = 0.0;
      for (j = 0; j < local_n; j++)
         partial_y[i] += local_A[i*local_n + j] * local_x[j];
   }

   for (i = 0; i < comm_sz; i++)
      recv_counts[i] = local_n;

   MPI_Reduce_scatter(partial_y, local_y, recv_counts, MPI_DOUBLE, MPI_SUM, comm);

   free(partial_y);
   free(recv_counts);
}  /* Mat_vect_mult */
