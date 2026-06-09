/* File:    count_sort.c
 *
 * Purpose: Use count_sort sort to sort a list of ints.
 *
 * Compile: gcc -g -Wall -o count_sort count_sort.c
 * Usage:   count_sort <n> <g|i>
 *             n:   number of elements in list
 *            'g':  generate list using a random number generator
 *            'i':  user input list
 *
 * Input:   list (optional)
 * Output:  sorted list
 *
 * IPP:
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* For random list, 0 <= keys < RMAX */
const int RMAX = 100;

void Usage(char *prog_name);
void Get_args(int argc, char *argv[], int *n_p, char *g_i_p);
void Generate_list(int a[], int n);
void Print_list(int a[], int n, char *title);
void Read_list(int a[], int n);
void Calculate_elapsed(clock_t start_time, clock_t end_time);
void Count_sort(int a[], int n);

/*-----------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int n;
    char g_i;
    int *a;
    clock_t start_time, end_time;

    Get_args(argc, argv, &n, &g_i);
    a = (int *)malloc(n * sizeof(int));
    if (g_i == 'g')
    {
        Generate_list(a, n);
        Print_list(a, n, "Before sort");
    }
    else
    {
        Read_list(a, n);
    }

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
    fprintf(stderr, "usage:   %s <n> <g|i>\n", prog_name);
    fprintf(stderr, "   n:   number of elements in list\n");
    fprintf(stderr, "  'g':  generate list using a random number generator\n");
    fprintf(stderr, "  'i':  user input list\n");
} /* Usage */

/*-----------------------------------------------------------------
 * Function:  Get_args
 * Purpose:   Get and check command line arguments
 * In args:   argc, argv
 * Out args:  n_p, g_i_p
 */
void Get_args(int argc, char *argv[], int *n_p, char *g_i_p)
{
    if (argc != 3)
    {
        Usage(argv[0]);
        exit(0);
    }
    *n_p = atoi(argv[1]);
    *g_i_p = argv[2][0];

    if (*n_p <= 0 || (*g_i_p != 'g' && *g_i_p != 'i'))
    {
        Usage(argv[0]);
        exit(0);
    }
} /* Get_args */

/*-----------------------------------------------------------------
 * Function:  Generate_list
 * Purpose:   Use random number generator to generate list elements
 * In args:   n
 * Out args:  a
 */
void Generate_list(int a[], int n)
{
    int i;

    srandom(0);
    for (i = 0; i < n; i++)
        a[i] = random() % RMAX;
} /* Generate_list */

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
 * Purpose:   Read elements of list from stdin
 * In args:   n
 * Out args:  a
 */
void Read_list(int a[], int n)
{
    int i;

    printf("Please enter the elements of the list\n");
    for (i = 0; i < n; i++)
        scanf("%d", &a[i]);
} /* Read_list */

void Calculate_elapsed(clock_t start_time, clock_t end_time)
{
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Function took %f seconds to execute.\n", cpu_time_used);
}

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
}
/* Count_sort */