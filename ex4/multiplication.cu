//Rotem Beker 217386598
#include <stdio.h>
#include <cuda_runtime.h>

#define N 4

// CUDA kernel for matrix multiplication
// Each thread computes one element of the result matrix
__global__ void matrixMul(int *A, int *B, int *C, int size) {
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    if (row < size && col < size) {
        int sum = 0;
        for (int k = 0; k < size; k++) {
            sum += A[row * size + k] * B[k * size + col];
        }
        C[row * size + col] = sum;
    }
}

// Helper function to print a matrix
void printMatrix(int *M, int size, const char *name) {
    printf("%s:\n", name);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%d", M[i * size + j]);
            if (j < size - 1) printf(" ");
        }
        printf("\n");
    }
}

int main() {
    // Host matrices
    int A[16] = {1, 2, 3, 4,
                 5, 6, 7, 8,
                 9, 10, 11, 12,
                 13, 14, 15, 16};

    int B[16] = {2, 4, 6, 8,
                 10, 12, 14, 16,
                 18, 20, 22, 24,
                 26, 28, 30, 32};

    int C[16];

    // Device pointers
    int *d_A, *d_B, *d_C;
    size_t size = N * N * sizeof(int);

    // Allocate device memory
    cudaMalloc(&d_A, size);
    cudaMalloc(&d_B, size);
    cudaMalloc(&d_C, size);

    // Copy matrices from host to device
    cudaMemcpy(d_A, A, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, B, size, cudaMemcpyHostToDevice);

    // Define block and grid dimensions
    // Each thread computes one element, so we need N x N threads
    dim3 threadsPerBlock(N, N);
    dim3 numBlocks(1, 1);

    // Launch kernel
    matrixMul<<<numBlocks, threadsPerBlock>>>(d_A, d_B, d_C, N);

    // Copy result from device to host
    cudaMemcpy(C, d_C, size, cudaMemcpyDeviceToHost);

    // Print results
    printMatrix(A, N, "Matrix A");
    printf("\n");
    printMatrix(B, N, "Matrix B");
    printf("\n");
    printMatrix(C, N, "Matrix C (A * B)");

    // Free device memory
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    return 0;
}
