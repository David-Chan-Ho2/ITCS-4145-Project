/* File:    count_sort_parallel.c
 *
 * Purpose: Use count_sort sort to sort a list of ints using OpenMP.
 *
 * Compile: gcc -g -Wall -fopenmp -o count_sort_parallel count_sort_parallel.c
 * Usage:   count_sort_parallel <thread_count> <input_file>
 *             thread_count: number of OpenMP threads to use
 *             input_file: text file with one integer per line
 *
 * Input:   input file
 * Output:  sorted list
 *
 * IPP: Programming Problem 5.3 (p. 287)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>

void Usage(char *prog_name);
void Print_list(int a[], int n, char *title);
void Read_list(FILE *fp, int a[], int n);
void Calculate_elapsed(double start_time, double end_time);
void Count_sort(int a[], int n);

/*-----------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int n, thread_count;
    int *a;
    double start_time, end_time;
    FILE *fp;

    if (argc != 3)
    {
        Usage(argv[0]);
        exit(0);
    }

    thread_count = atoi(argv[1]);
    omp_set_num_threads(thread_count);

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "../../input-files/%s", argv[2]);
    fp = fopen(filepath, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error: cannot open file %s\n", filepath);
        exit(1);
    }

    /* count lines to determine n */
    int ch;
    n = 0;
    while ((ch = fgetc(fp)) != EOF)
        if (ch == '\n')
            n++;
    rewind(fp);

    a = (int *)malloc(n * sizeof(int));
    Read_list(fp, a, n);
    fclose(fp);

    Print_list(a, n, "Before sort");

    start_time = omp_get_wtime();
    Count_sort(a, n);
    end_time = omp_get_wtime();

    Print_list(a, n, "After sort");
    Calculate_elapsed(start_time, end_time);

    free(a);
    return 0;
} /* main */

/*-----------------------------------------------------------------
 * Function:  Usage
 * Purpose:   Summary of how to run program
 */
void Usage(char *prog_name)
{
    fprintf(stderr, "usage:   %s <thread_count> <input_file>\n", prog_name);
    fprintf(stderr, "   thread_count: number of OpenMP threads to use\n");
    fprintf(stderr, "   input_file: text file with one integer per line\n");
} /* Usage */

/*-----------------------------------------------------------------
 * Function:  Print_list
 * Purpose:   Print the elements in the list
 * In args:   a, n
 */
void Print_list(int a[], int n, char *title)
{
    int i;

    printf("%s:\n", title);
    for (i = 0; i < n; i++)
        printf("%d ", a[i]);
    printf("\n\n");
} /* Print_list */

/*-----------------------------------------------------------------
 * Function:  Read_list
 * Purpose:   Read elements of list from file
 * In args:   fp, n
 * Out args:  a
 */
void Read_list(FILE *fp, int a[], int n)
{
    int i;

    for (i = 0; i < n; i++)
        fscanf(fp, "%d", &a[i]);
} /* Read_list */

/*-----------------------------------------------------------------
 * Function:     Calculate_elapsed
 * Purpose:      Calculate and display the wall-clock time used by a function
 * In args:      start_time, end_time
 */
void Calculate_elapsed(double start_time, double end_time)
{
    printf("Function took %f seconds to execute.\n", end_time - start_time);
} /* Calculate_elapsed */

/*-----------------------------------------------------------------
 * Function:     Count_sort
 * Purpose:      Sort list using count sort
 * In args:      n
 * In/out args:  a
 */
void Count_sort(int a[], int n)
{
    int i, j, count;
    int *temp = malloc(n * sizeof(int));

#   pragma omp parallel for private(j, count)
    for (i = 0; i < n; i++)
    {
        count = 0;

        for (j = 0; j < n; j++)
            if (a[j] < a[i])
                count++;
            else if (a[j] == a[i] && j < i)
                count++;

        temp[count] = a[i];
    }

    memcpy(a, temp, n * sizeof(int));
    free(temp);
} /* Count_sort */
