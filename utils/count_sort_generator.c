#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int RMAX = 100;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <num_count>\n", argv[0]);
        return 1;
    }

    int nums = atoi(argv[1]);
    if (nums <= 0)
    {
        fprintf(stderr, "num_count must be a positive integer\n");
        return 1;
    }

    char file_path[256];
    snprintf(file_path, sizeof(file_path), "../input-files/input-%d.txt", nums);

    FILE *file = fopen(file_path, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Error opening file: %s\n", file_path);
        return 1;
    }

    srand(time(NULL));

    for (int i = 0; i < nums; i++)
    {
        fprintf(file, "%d\n", rand() % RMAX);
    }

    fclose(file);
    printf("Generated %d numbers in %s\n", nums, file_path);

    return 0;
}