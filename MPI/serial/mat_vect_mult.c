/* File:     mat_vect_mult.c
 *
 * Purpose:  Implement serial matrix-vector multiplication using
 *           one-dimensional arrays to store the vectors and the
 *           matrix.
 *
 * Compile:  gcc -g -Wall -o mat_vect_mult mat_vect_mult.c
 * Usage:    ./mat_vect_mult <input_file>
 *             input_file: text file with m, n, matrix A, vector x
 *
 * Input:    Dimensions of the matrix (m = number of rows, n
 *              = number of columns)
 *           n-dimensional vector x
 * Output:   Product vector y = Ax
 *
 * Errors:   if the number of user-input rows or column isn't
 *           positive, the program prints a message and quits.
 * Note:     Define DEBUG for verbose output
 *
 * IPP:      Section 3.4.9 (pp. 113 and ff.), Section 4.3 (pp. 159
 *           and ff.), and Section 5.9 (pp. 252 and ff.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void Usage(char *prog_name);
void Get_dims(int *m_p, int *n_p, FILE *fp);
void Read_matrix(char prompt[], double A[], int m, int n, FILE *fp);
void Read_vector(char prompt[], double x[], int n, FILE *fp);
void Print_matrix(char title[], double A[], int m, int n);
void Print_vector(char title[], double y[], int m);
void Calculate_elapsed(clock_t start_time, clock_t end_time);
void Mat_vect_mult(double A[], double x[], double y[], int m, int n);

/*-------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
   double *A = NULL;
   double *x = NULL;
   double *y = NULL;
   int m, n;
   clock_t start_time, end_time;
   FILE *fp;

   if (argc != 2)
   {
      Usage(argv[0]);
      exit(0);
   }

   char filepath[512];
   snprintf(filepath, sizeof(filepath), "../../input-files/%s", argv[1]);
   fp = fopen(filepath, "r");
   if (fp == NULL)
   {
      fprintf(stderr, "Error: cannot open file %s\n", filepath);
      exit(1);
   }

   Get_dims(&m, &n, fp);
   A = malloc(m * n * sizeof(double));
   x = malloc(n * sizeof(double));
   y = malloc(m * sizeof(double));
   if (A == NULL || x == NULL || y == NULL)
   {
      fprintf(stderr, "Can't allocate storage\n");
      exit(-1);
   }
   Read_matrix("A", A, m, n, fp);
#ifdef DEBUG
   Print_matrix("A", A, m, n);
#endif
   Read_vector("x", x, n, fp);
#ifdef DEBUG
   Print_vector("x", x, n);
#endif

   fclose(fp);

   start_time = clock();
   Mat_vect_mult(A, x, y, m, n);
   end_time = clock();

   Print_vector("y", y, m);

   Calculate_elapsed(start_time, end_time);

   free(A);
   free(x);
   free(y);
   return 0;
} /* main */

/*-------------------------------------------------------------------
 * Function:   Get_dims
 * Purpose:    Read the dimensions of the matrix from stdin
 * Out args:   m_p:  number of rows
 *             n_p:  number of cols
 *
 * Errors:     If one of the dimensions isn't positive, the program
 *             prints an error and quits
 */
void Usage(char *prog_name)
{
   fprintf(stderr, "usage:   %s <input_file>\n", prog_name);
   fprintf(stderr, "   input_file: text file with m, n, matrix A, vector x\n");
} /* Usage */

/*-------------------------------------------------------------------*/
void Get_dims(
    int *m_p /* out */,
    int *n_p /* out */,
    FILE *fp /* in  */)
{
   fscanf(fp, "%d", m_p);
   fscanf(fp, "%d", n_p);

   if (*m_p <= 0 || *n_p <= 0)
   {
      fprintf(stderr, "m and n must be positive\n");
      exit(-1);
   }
} /* Get_dims */

/*-------------------------------------------------------------------
 * Function:   Read_matrix
 * Purpose:    Read the contents of the matrix from stdin
 * In args:    prompt:  description of matrix
 *             m:       number of rows
 *             n:       number of cols
 * Out arg:    A:       the matrix
 */
void Read_matrix(
    char prompt[] /* in  */,
    double A[] /* out */,
    int m /* in  */,
    int n /* in  */,
    FILE *fp /* in  */)
{
   int i, j;

   for (i = 0; i < m; i++)
      for (j = 0; j < n; j++)
         fscanf(fp, "%lf", &A[i * n + j]);
} /* Read_matrix */

/*-------------------------------------------------------------------*/
void Read_vector(
    char prompt[] /* in  */,
    double x[] /* out */,
    int n /* in  */,
    FILE *fp /* in  */)
{
   int i;

   for (i = 0; i < n; i++)
      fscanf(fp, "%lf", &x[i]);
} /* Read_vector */

/*-------------------------------------------------------------------
 * Function:   Print_matrix
 * Purpose:    Print a matrix to stdout
 * In args:    title:  title for output
 *             A:      the matrix
 *             m:      number of rows
 *             n:      number of cols
 */
void Print_matrix(
    char title[] /* in */,
    double A[] /* in */,
    int m /* in */,
    int n /* in */)
{
   int i, j;

   printf("\nThe matrix %s\n", title);
   for (i = 0; i < m; i++)
   {
      for (j = 0; j < n; j++)
         printf("%f ", A[i * n + j]);
      printf("\n");
   }
} /* Print_matrix */

/*-------------------------------------------------------------------
 * Function:   Print_vector
 * Purpose:    Print the contents of a vector to stdout
 * In args:    title:  title for output
 *             y:      the vector to be printed
 *             m:      the number of elements in the vector
 */
void Print_vector(
    char title[] /* in */,
    double y[] /* in */,
    int m /* in */)
{
   int i;

   printf("\nThe vector %s\n", title);
   for (i = 0; i < m; i++)
      printf("%f ", y[i]);
   printf("\n");
} /* Print_vector */

/*-----------------------------------------------------------------
 * Function:     Calculate_elapsed
 * Purpose:      Calculate and display the CPU time used by a function
 * In args:      start_time, end_time
 */
void Calculate_elapsed(clock_t start_time, clock_t end_time)
{
   double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
   printf("Function took %f seconds to execute.\n", cpu_time_used);
} /* Calculate_elapsed */

/*-------------------------------------------------------------------
 * Function:   Mat_vect_mult
 * Purpose:    Multiply a matrix by a vector
 * In args:    A: the matrix
 *             x: the vector being multiplied by A
 *             m: the number of rows in A and components in y
 *             n: the number of columns in A components in x
 * Out args:   y: the product vector Ax
 */
void Mat_vect_mult(
    double A[] /* in  */,
    double x[] /* in  */,
    double y[] /* out */,
    int m /* in  */,
    int n /* in  */)
{
   int i, j;

   for (i = 0; i < m; i++)
   {
      y[i] = 0.0;
      for (j = 0; j < n; j++)
         y[i] += A[i * n + j] * x[j];
   }
} /* Mat_vect_mult */
