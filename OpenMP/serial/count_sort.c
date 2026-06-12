/* File:    count_sort.c
 *
 * Purpose: Use count_sort sort to sort a list of ints.
 *
 * Compile: gcc -g -Wall -o count_sort count_sort.c
 * Usage:   count_sort <input_file>
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
#include <time.h>

void Usage(char *prog_name);
void Print_list(int a[], int n, char *title);
void Read_list(FILE *fp, int a[], int n);
void Calculate_elapsed(clock_t start_time, clock_t end_time);
void Count_sort(int a[], int n);

/*-----------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int n;
    int *a;
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

    start_time = clock();
    Count_sort(a, n);
    end_time = clock();

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
    fprintf(stderr, "usage:   %s <input_file>\n", prog_name);
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
 * Purpose:      Calculate and display the CPU time used by a function
 * In args:      start_time, end_time
 */
void Calculate_elapsed(clock_t start_time, clock_t end_time)
{
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Function took %f seconds to execute.\n", cpu_time_used);
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