#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERIAL_DIR="$SCRIPT_DIR/../OpenMP/serial"
PARALLEL_DIR="$SCRIPT_DIR/../OpenMP/parallel"
INPUT_FILES=("input-1000.txt" "input-2000.txt" "input-4000.txt" "input-8000.txt" "input-10000.txt")
THREAD_COUNTS=(2 4 8)
RESULTS="$SCRIPT_DIR/count_sort_results.txt"

# Compile
echo "Compiling..."
(cd "$SERIAL_DIR" && gcc -g -Wall -o count_sort count_sort.c)
(cd "$PARALLEL_DIR" && gcc -g -Wall -fopenmp -o count_sort_parallel count_sort_parallel.c)
echo ""

# Init results file
echo "Count Sort Execution Times — $(date)" > "$RESULTS"
echo "========================================" >> "$RESULTS"

# Serial runs
echo "========================================"
echo " Serial Count Sort"
echo "========================================"
echo "" >> "$RESULTS"
echo "Serial" >> "$RESULTS"
echo "----------------------------------------" >> "$RESULTS"

for f in "${INPUT_FILES[@]}"; do
    echo "--- $f ---"
    output=$(cd "$SERIAL_DIR" && ./count_sort "$f")
    echo "$output"
    echo ""
    elapsed=$(echo "$output" | grep "Function took")
    echo "  $f: $elapsed" >> "$RESULTS"
done

# Parallel runs
echo "========================================"
echo " Parallel Count Sort"
echo "========================================"

for t in "${THREAD_COUNTS[@]}"; do
    echo "--- Threads: $t ---"
    echo "" >> "$RESULTS"
    echo "Parallel — $t threads" >> "$RESULTS"
    echo "----------------------------------------" >> "$RESULTS"
    for f in "${INPUT_FILES[@]}"; do
        echo "  [ $f ]"
        output=$(cd "$PARALLEL_DIR" && ./count_sort_parallel "$t" "$f")
        echo "$output"
        echo ""
        elapsed=$(echo "$output" | grep "Function took")
        echo "  $f: $elapsed" >> "$RESULTS"
    done
done

echo ""
echo "Execution times saved to: $RESULTS"
