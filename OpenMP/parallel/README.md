clang -Xpreprocessor -fopenmp \
 -I$(brew --prefix libomp)/include \
  count_sort_parallel.c \
  -L$(brew --prefix libomp)/lib \
 -lomp \
 -o count_sort_parallel

./count_sort_parallel <thread_count> <input_file>
# example: ./count_sort_parallel 4 input-1000.txt
