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
    ├── run_count_sort.sh         # Build + benchmark script (OpenMP)
    ├── run_mat_vect.sh           # Build + benchmark script (MPI)
    ├── count_sort_results.txt    # Count sort timing results
    └── mat_vect_results.txt      # Matrix-vector timing results
```

---

## Dependencies

- GCC with OpenMP support (`-fopenmp`)
- MPI implementation — OpenMPI or MPICH (`mpicc`, `mpiexec`)

---

## Running the Benchmarks

Both scripts compile the programs, run all input sizes, and write timing results to `utils/`.

**Count Sort (OpenMP)**

```bash
cd utils
./run_count_sort.sh
# Output: utils/count_sort_results.txt
```

Runs serial and parallel (2, 4, 8 threads) against inputs of 1 000 – 10 000 integers.

**Matrix-Vector Multiplication (MPI)**

```bash
cd utils
./run_mat_vect.sh
# Output: utils/mat_vect_results.txt
```

Compiles and runs serial, row-block MPI, and column-block MPI against matrices of size 512 – 4 096. 
Parallel runs use 2, 4, and 8 MPI processes.

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

