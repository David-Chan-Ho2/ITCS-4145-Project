#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Generates a mat-vect input file with an m x m matrix and m-dimensional
 * vector filled with random doubles in [0, 10).
 * File format:
 *   m m
 *   <m*m matrix values, one row per line>
 *   <m vector values>
 * m must be divisible by 8 so it works with 1, 2, 4, and 8 MPI processes.
 */

const double RMAX = 10.0;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <size>\n", argv[0]);
        fprintf(stderr, "   size: matrix dimension (must be divisible by 8)\n");
        return 1;
    }

    int m = atoi(argv[1]);
    if (m <= 0 || m % 8 != 0)
    {
        fprintf(stderr, "size must be a positive integer divisible by 8\n");
        return 1;
    }

    char file_path[256];
    snprintf(file_path, sizeof(file_path), "../input-files/mat-vect-%d.txt", m);

    FILE *file = fopen(file_path, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", file_path);
        return 1;
    }

    srand(time(NULL));

    fprintf(file, "%d %d\n", m, m);

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < m; j++)
        {
            double val = ((double)rand() / RAND_MAX) * RMAX;
            fprintf(file, "%.4f", val);
            if (j < m - 1) fprintf(file, " ");
        }
        fprintf(file, "\n");
    }

    for (int i = 0; i < m; i++)
    {
        double val = ((double)rand() / RAND_MAX) * RMAX;
        fprintf(file, "%.4f", val);
        if (i < m - 1) fprintf(file, " ");
    }
    fprintf(file, "\n");

    fclose(file);
    printf("Generated %dx%d mat-vect input in %s\n", m, m, file_path);

    return 0;
}
