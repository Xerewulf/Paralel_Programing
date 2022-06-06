#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// CUDA kernel. Each thread takes care of one element of c
__global__ void vecAdd(double *a, double *b, double *c, int n)
{
    // Get our global thread ID
    int id = blockIdx.x*blockDim.x+threadIdx.x;
 
    // Make sure we do not go out of bounds
    if (id < n)
        c[id] = a[id] + b[id];
}

void Serial_sum(double *a, double *b, double cz[], int n)
{

for(int i=0;i<n;i++){
cz[i]= a[i]+b[i];
 }

}

double twoNormDiff(double *c,double *cz, int n)
{
double sum=0;
for(int i=0;i<n;i++){
	sum += pow(c[i],2)-pow(cz[i],2);
    }
return sqrt(sum);
}





int main( int argc, char* argv[] )
{
    // Size of vectors
    int n = 500000;
 
    // Host input vectors
    double *h_a;
    double *h_b;
    //Host output vector
    double *h_c;
    double *cz;
    // Device input vectors
    double *d_a;
    double *d_b;
    //Device output vector
    double *d_c;

    // Size, in bytes, of each vector
    size_t bytes = n*sizeof(double);

    // Allocate memory for each vector on host
    h_a = (double*)malloc(bytes);
    h_b = (double*)malloc(bytes);
    h_c = (double*)malloc(bytes);
    cz  = (double*)malloc(bytes);
    // Allocate memory for each vector on GPU
    cudaMalloc(&d_a, bytes);
    cudaMalloc(&d_b, bytes);
    cudaMalloc(&d_c, bytes);

    int i;
    // Initialize vectors on host
    for( i = 0; i < n; i++ ) {
        h_a[i] = (rand()%50000)*sin(i);
        h_b[i] = (rand()%50000)*cos(i);
    }
 
    // Copy host vectors to device
    cudaMemcpy( d_a, h_a, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy( d_b, h_b, bytes, cudaMemcpyHostToDevice);
 
    int blockSize, gridSize;
 
    // Number of threads in each thread block
    blockSize = strtol(argv[1], NULL, 10);
    gridSize  = strtol(argv[2], NULL, 10);

    // Number of thread blocks in grid
    
 
    // Execute the kernel
    vecAdd<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);
 
    // Copy array back to host
    cudaMemcpy( h_c, d_c, bytes, cudaMemcpyDeviceToHost );
    Serial_sum(d_a, d_b, cz, n);
    printf("two norm of diffence between howstt and device is: %lf", twoNormDiff(h_c, cz, n));
 
    // Release device memory
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
 
    // Release host memory
    free(h_a);
    free(h_b);
    free(h_c);
 
    return 0;
}
