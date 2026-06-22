//Rotem Beker 217386598

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "matrix.h"

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 5) {
        if (rank == 0) {
            fprintf(stderr, "Usage: mpirun -np <P> ./matmul <N> <seedA> <seedB> <maxValue>\n");
        }
        MPI_Finalize();
        return 1;
    }

    int N = atoi(argv[1]);
    uint64_t seedA = (uint64_t)atoll(argv[2]);
    uint64_t seedB = (uint64_t)atoll(argv[3]);
    int maxValue = atoi(argv[4]);

    // Calculate row distribution for each process
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs = (int *)malloc(size * sizeof(int));

    for (int r = 0; r < size; r++) {
        int start_row = (r * N) / size;
        int end_row = ((r + 1) * N) / size;
        int num_rows = end_row - start_row;
        sendcounts[r] = num_rows * N;
        displs[r] = start_row * N;
    }

    int my_start_row = (rank * N) / size;
    int my_end_row = ((rank + 1) * N) / size;
    int my_num_rows = my_end_row - my_start_row;

    // Allocate local buffers
    int *local_A = (int *)malloc(my_num_rows * N * sizeof(int));
    int *B = (int *)malloc(N * N * sizeof(int));
    int *local_C = (int *)malloc(my_num_rows * N * sizeof(int));

    IMatrix A_full = {0, NULL};
    IMatrix C_full = {0, NULL};

    if (rank == 0) {
        // Generate matrices A and B
        A_full = imatrix_alloc(N);
        IMatrix B_mat = imatrix_alloc(N);
        C_full = imatrix_alloc(N);

        imatrix_fill_random(&A_full, seedA, maxValue);
        imatrix_fill_random(&B_mat, seedB, maxValue);

        // Copy B data to flat array for broadcast
        for (int i = 0; i < N * N; i++) {
            B[i] = B_mat.data[i];
        }

        imatrix_free(&B_mat);
    }

    // Broadcast B to all processes
    MPI_Bcast(B, N * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Scatter rows of A to all processes
    MPI_Scatterv(rank == 0 ? A_full.data : NULL, sendcounts, displs, MPI_INT,
                 local_A, my_num_rows * N, MPI_INT, 0, MPI_COMM_WORLD);

    // Compute local block of C using standard triple-loop algorithm
    for (int i = 0; i < my_num_rows; i++) {
        for (int j = 0; j < N; j++) {
            int sum = 0;
            for (int k = 0; k < N; k++) {
                sum += local_A[i * N + k] * B[k * N + j];
            }
            local_C[i * N + j] = sum;
        }
    }

    // Gather results back to rank 0
    MPI_Gatherv(local_C, my_num_rows * N, MPI_INT,
                rank == 0 ? C_full.data : NULL, sendcounts, displs, MPI_INT,
                0, MPI_COMM_WORLD);

    // Rank 0 prints checksum
    if (rank == 0) {
        long long checksum = imatrix_checksum(&C_full);
        printf("checksum (C)=<%lld>\n", checksum);

        imatrix_free(&A_full);
        imatrix_free(&C_full);
    }

    free(local_A);
    free(B);
    free(local_C);
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}
