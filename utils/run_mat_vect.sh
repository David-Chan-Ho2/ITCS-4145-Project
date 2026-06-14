#!/bin/bash
# run_mat_vect.sh
# Compiles and benchmarks serial, row-block, and column-block matrix-vector
# multiplication against every mat-vect-*.txt input file.
# Parallel runs use 2, 4, and 8 MPI processes.
# Results are printed to stdout and written to results.txt.
#
# Usage: ./run_mat_vect.sh

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERIAL_DIR="$SCRIPT_DIR/../MPI/serial"
PARALLEL_DIR="$SCRIPT_DIR/../MPI/parallel"
UTILS_DIR="$SCRIPT_DIR"
INPUT_DIR="$(cd "$SCRIPT_DIR/../input-files" && pwd)"
OUTPUT_FILE="$SCRIPT_DIR/mat_vect_results.txt"
TMP_MAT=$(mktemp /tmp/mat_XXXXXX.txt)
TMP_VEC=$(mktemp /tmp/vec_XXXXXX.txt)
trap 'rm -f "$TMP_MAT" "$TMP_VEC"' EXIT

# --- compile ---
echo "=== Compiling ==="
gcc   -g -Wall -O2 -o "$UTILS_DIR/mat_vect_generator"        "$UTILS_DIR/mat_vect_generator.c"        && echo "  mat_vect_generator    OK"
gcc   -g -Wall -O2 -o "$SERIAL_DIR/mat_vect_mult"            "$SERIAL_DIR/mat_vect_mult.c"            && echo "  mat_vect_mult         OK"
mpicc -g -Wall -O2 -o "$PARALLEL_DIR/mpi_mat_vect_time"      "$PARALLEL_DIR/mpi_mat_vect_time.c"      && echo "  mpi_mat_vect_time     OK"
mpicc -g -Wall -O2 -o "$PARALLEL_DIR/mpi_col_mat_vect_time"  "$PARALLEL_DIR/mpi_col_mat_vect_time.c"  && echo "  mpi_col_mat_vect_time OK"
echo

# --- generate large input files if missing ---
echo "=== Generating input files ==="
for SIZE in 512 1024 2048 4096; do
    FILE="$INPUT_DIR/mat-vect-$SIZE.txt"
    if [ ! -f "$FILE" ]; then
        (cd "$UTILS_DIR" && ./mat_vect_generator "$SIZE")
    else
        echo "  mat-vect-$SIZE.txt already exists, skipping"
    fi
done
echo

# --- benchmark (all output goes to stdout and results.txt) ---
{
    echo "Results - Matrix-Vector Multiplication Benchmark"
    echo "Generated: $(date '+%Y-%m-%d %H:%M:%S')"
    echo "================================================="
    echo

    for INPUT_FILE in "$INPUT_DIR"/mat-vect-*.txt; do
        BASENAME=$(basename "$INPUT_FILE")
        read -r M N < "$INPUT_FILE"

        echo "Input: $BASENAME (${M}x${N})"

        # split combined file into separate matrix and vector temp files
        head -n $((M + 1)) "$INPUT_FILE" > "$TMP_MAT"
        tail -n 1          "$INPUT_FILE" > "$TMP_VEC"

        # serial: run from serial dir so its relative path to input-files resolves
        SERIAL_OUT=$(cd "$SERIAL_DIR" && ./mat_vect_mult "$BASENAME" 2>&1 | grep "Function took")
        printf "  %-22s %s\n" "Serial:" "$SERIAL_OUT"

        for NP in 2 4 8; do
            ROW_OUT=$(mpiexec -n "$NP" "$PARALLEL_DIR/mpi_mat_vect_time"     "$TMP_MAT" "$TMP_VEC" 2>&1)
            COL_OUT=$(mpiexec -n "$NP" "$PARALLEL_DIR/mpi_col_mat_vect_time" "$TMP_MAT" "$TMP_VEC" 2>&1)
            printf "  %-22s %s\n" "Row-block ($NP procs):" "$ROW_OUT"
            printf "  %-22s %s\n" "Col-block ($NP procs):" "$COL_OUT"
        done

        echo
    done
} | tee "$OUTPUT_FILE"

echo "Results saved to $OUTPUT_FILE"
