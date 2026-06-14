# ITCS-4145 Parallel Programming — Project

Benchmarks two classic algorithms under two different parallel programming models:

| Algorithm | Model | Parallelism |
|---|---|---|
| Count Sort | OpenMP | Shared-memory threads |
| Matrix-Vector Multiplication | MPI | Distributed-memory processes |

---

## Repository Structure

```
ITCS-4145-Project/
├── OpenMP/
│   ├── serial/
│   │   └── count_sort.c              # Serial count sort
│   └── parallel/
│       └── count_sort_parallel.c     # OpenMP parallel count sort
├── MPI/
│   ├── serial/
│   │   └── mat_vect_mult.c           # Serial matrix-vector multiply
│   ├── parallel/
│   │   ├── mpi_mat_vect_time.c       # MPI row-block distribution
│   │   └── mpi_col_mat_vect_time.c   # MPI column-block distribution
│   └── timer.h
├── input-files/
│   ├── input-{1000,2000,4000,8000,10000}.txt   # Count sort inputs
│   └── mat-vect-{512,1024,2048,4096}.txt        # Matrix-vector inputs
└── utils/
    ├── count_sort_generator.c    # Generates count sort input files
    ├── mat_vect_generator.c      # Generates matrix-vector input files
```

---

## Dependencies

- GCC with OpenMP support (`-fopenmp`)
- MPI implementation — OpenMPI or MPICH (`mpicc`, `mpiexec`)

---

## Running the Benchmarks

**OpenMP/serial**

```bash
cd OpenMP/serial
gcc -g -Wall -O2 -o count_sort count_sort.c

# input file path is relative to OpenMP/serial/ (../../input-files/<file>)
./count_sort input-1000.txt
```

**OpenMP/parallel**

```bash
cd OpenMP/parallel
gcc -g -Wall -O2 -fopenmp -o count_sort_parallel count_sort_parallel.c

# usage: ./count_sort_parallel <thread_count> <input_file>
./count_sort_parallel 4 input-1000.txt
```

Replace `4` with 2 or 8; replace `input-1000.txt` with any file from `input-files/`.

**MPI/serial**

```bash
cd MPI/serial
gcc -g -Wall -O2 -o mat_vect_mult mat_vect_mult.c

# input file path is relative to MPI/serial/ (../../input-files/<file>)
./mat_vect_mult mat-vect-1024.txt
```

**MPI/parallel**

The parallel programs take the matrix and vector as *separate* files.
Split a combined input file first:

```bash
INPUT=input-files/mat-vect-1024.txt
M=$(head -1 "$INPUT" | awk '{print $1}')

head -n $((M + 1)) "$INPUT" > /tmp/mat.txt
tail -n 1          "$INPUT" > /tmp/vec.txt
```

Row-block distribution:

```bash
cd MPI/parallel
mpicc -g -Wall -O2 -o mpi_mat_vect_time mpi_mat_vect_time.c

mpiexec -n 4 ./mpi_mat_vect_time /tmp/mat.txt /tmp/vec.txt
```

Column-block distribution:

```bash
mpicc -g -Wall -O2 -o mpi_col_mat_vect_time mpi_col_mat_vect_time.c

mpiexec -n 4 ./mpi_col_mat_vect_time /tmp/mat.txt /tmp/vec.txt
```

Replace `-n 4` with 2 or 8; the process count must evenly divide the matrix dimension.

## Input File Formats

**Count sort** — one integer per line:
```
4821
3917
...
```

**Matrix-vector** — dimensions on the first line, matrix rows, then the vector:
```
1024 1024
0.512 0.831 ...
...
0.213 0.447 ...
```

To generate new inputs manually:
```bash
cd utils
gcc -O2 -o count_sort_generator count_sort_generator.c && ./count_sort_generator <n>
gcc -O2 -o mat_vect_generator   mat_vect_generator.c   && ./mat_vect_generator <n>
```

