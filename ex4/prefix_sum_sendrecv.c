//Rotem Beker 217386598

#include <mpi.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int x = rank;  // x_r = r
    int prefix;

    if (rank == 0) {
        // Process 0: prefix is just x_0
        prefix = x;
        // Send prefix to next process if it exists
        if (size > 1) {
            MPI_Send(&prefix, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
        }
    } else {
        // Receive prefix sum from previous process
        int received_prefix;
        MPI_Recv(&received_prefix, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Compute this process's prefix sum
        prefix = received_prefix + x;

        // Send to next process if it exists
        if (rank < size - 1) {
            MPI_Send(&prefix, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
        }
    }

    // Each rank prints its result
    printf("rank=<%d> x=<%d> prefix=<%d>\n", rank, x, prefix);

    MPI_Finalize();
    return 0;
}
