#include <stdio.h>
#include <omp.h>
#include <stdlib.h>
#include <time.h>

#define DIM 1000 // Define matrix dimension

void matrix_multiply_single(int dim, double **a, double **b, double **c) {
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            c[i][j] = 0;
            for (int k = 0; k < dim; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void matrix_multiply_outer_loop_parallel(int dim, double **a, double **b, double **c) {
    #pragma omp parallel for
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            c[i][j] = 0;
            for (int k = 0; k < dim; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void matrix_multiply_outer_two_loops_parallel(int dim, double **a, double **b, double **c) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            c[i][j] = 0;
            for (int k = 0; k < dim; k++) {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

void matrix_multiply_all_loops_parallel(int dim, double **a, double **b, double **c) {
    #pragma omp parallel for collapse(3)
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            for (int k = 0; k < dim; k++) {
                #pragma omp atomic
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
}

// allocation of matrices
double **allocate_matrix(int dim) {
    double **matrix = malloc(dim * sizeof(double *));
    for (int i = 0; i < dim; i++) {
        matrix[i] = malloc(dim * sizeof(double));
    }
    return matrix;
}

// freeing memory from matrices
void free_matrix(double **matrix, int dim) {
    for (int i = 0; i < dim; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main(int argc, char *argv[]) {
    int dim = DIM;
    int num_threads = 4;
    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }

    // Allocate matrices
    double **a = allocate_matrix(dim);
    double **b = allocate_matrix(dim);
    double **c = allocate_matrix(dim);

    // Initialize matrices
    srand(time(0));
    for (int i = 0; i < dim; i++) {
        for (int j = 0; j < dim; j++) {
            a[i][j] = rand() % 100;
            b[i][j] = rand() % 100;
        }
    }

    // Set the number of threads
    omp_set_num_threads(num_threads);

    // Measure performance for each parallelization strategy
    double start_time, end_time;

    // Case 1: Only outermost loop parallelized
    start_time = omp_get_wtime();
    matrix_multiply_outer_loop_parallel(dim, a, b, c);
    end_time = omp_get_wtime();
    printf("Time for outermost loop parallelization: %f seconds\n", end_time - start_time);

    // Case 2: Outer two loops parallelized
    start_time = omp_get_wtime();
    matrix_multiply_outer_two_loops_parallel(dim, a, b, c);
    end_time = omp_get_wtime();
    printf("Time for outer two loops parallelization: %f seconds\n", end_time - start_time);

    // Case 3: All three loops parallelized
    start_time = omp_get_wtime();
    matrix_multiply_all_loops_parallel(dim, a, b, c);
    end_time = omp_get_wtime();
    printf("Time for all three loops parallelization: %f seconds\n", end_time - start_time);

    // Clean up memory
    free_matrix(a, dim);
    free_matrix(b, dim);
    free_matrix(c, dim);

    return 0;
}

