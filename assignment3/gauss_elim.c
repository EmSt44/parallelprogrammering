#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Function to generate an upper triangular matrix and a random vector
void generate_upper_triangular_matrix(int n, double** A, double* b) {
    for (int i = 0; i < n; i++) {
        b[i] = rand() % 100; // Random value for b[i]
        for (int j = 0; j < n; j++) {
            if (i <= j) {
                A[i][j] = (rand() % 100) + 1; // Random value for upper triangle
            } else {
                A[i][j] = 0; // Lower triangle should be zero
            }
        }
    }
}

// Row-Oriented algorithm parallelizing the inner loop
void row_oriented(int n, double** A, double* b, double* x) {
    int row, col;

    for (row = n-1; row >= 0; row--) {
        x[row] = b[row];
        
        #pragma omp parallel for schedule(runtime) shared(A, x, row) private(col)
        for (col = row+1; col < n; col++) {
            x[row] -= A[row][col] * x[col];
        }
        
        x[row] /= A[row][row];
    }
}

// Column-oriented algorithm parallelizing the inner loop on the second nested loop
void column_oriented(int n, double** A, double* b, double* x) {
    int row, col;
    
    for (row = 0; row < n; row++) {
        x[row] = b[row];
    }
    
    for (col = n-1; col >= 0; col--) {
        x[col] /= A[col][col];
        
        #pragma omp parallel for schedule(runtime) shared(A, x, col) private(row)
        for (row = 0; row < col; row++) {
            x[row] -= A[row][col] * x[col];
        }
    }
}

int main(int argc, char *argv[]) {
    int num_threads = 4; //if no argument is given run with default 4 threads
    if (argc > 1) {
        num_threads = atoi(argv[1]);
    }

    int n = 20000;

    // Dynamically allocate memory for the matrix A and vectors b and x
    double** A = (double**) malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        A[i] = (double*) malloc(n * sizeof(double));
    }
    double* b = (double*) malloc(n * sizeof(double));
    double* x = (double*) malloc(n * sizeof(double));
    double* cx = (double*) malloc(n * sizeof(double));

    // Initialize random seed
    srand(42);

    // Generate the upper triangular matrix A and vector b
    generate_upper_triangular_matrix(n, A, b);
    
    // Set the number of threads
    omp_set_num_threads(num_threads);

    // For measuring time on the different algorithms
    double start_time, end_time;

    // Solve the system using the row-oriented algorithm
    start_time = omp_get_wtime();
    row_oriented(n, A, b, x);
    end_time = omp_get_wtime();
    printf("Time for row-oriented algorithm: %f seconds\n", end_time - start_time);

    // Solve the system using the column-oriented algorithm
    start_time = omp_get_wtime();
    column_oriented(n, A, b, cx);
    end_time = omp_get_wtime();
    printf("Time for column-oriented algorithm: %f seconds\n", end_time - start_time);

    // Free dynamically allocated memory
    for (int i = 0; i < n; i++) {
        free(A[i]);
    }
    free(A);
    free(b);
    free(x);

    return 0;
}
