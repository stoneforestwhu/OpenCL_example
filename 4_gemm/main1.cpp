#include "util.h"

#define uint unsigned int

int main() {
  cl_int errNum;
  // platforms_0=cl_platform_id[1](0)
  cl_platform_id platforms_0[1];
  clGetPlatformIDs(1, platforms_0, NULL); //*num_platforms=2

  // devs_0=cl_device_id[1](0)
  cl_device_id devs_0[1];
  clGetDeviceIDs(platforms_0[0], CL_DEVICE_TYPE_GPU, 1, devs_0, NULL);

  cl_device_id devs_1[1] = {devs_0[0]};
  cl_context ctx_0 = clCreateContext(NULL, 1, devs_1, NULL, NULL, NULL);
  cl_command_queue cmdQue_0 = clCreateCommandQueue(ctx_0, devs_0[0], 0, NULL);

  std::shared_ptr<char> clProg_0_src_0_shared_ptr =
      readKernelFile("kernel/kernel1.txt");
  char *clProg_0_src_0 = clProg_0_src_0_shared_ptr.get();
  char *clProg_0_srcPtr_0[1] = {clProg_0_src_0};
  cl_program clProg_0 = clCreateProgramWithSource(
      ctx_0, 1, (const char **)clProg_0_srcPtr_0, NULL, NULL);
  clBuildProgram(clProg_0, 0, NULL, NULL, NULL, NULL);
  cl_kernel kernel_0 = clCreateKernel(clProg_0, "myGEMM1", NULL);

  const int M = 64;
  const int K = 32;
  const int N = 64;

  float A[M][K];
  int initVal = 0;
  for (unsigned int i = 0; i < M; ++i) {
    for (unsigned int j = 0; j < K; ++j) {
      A[i][j] = initVal++;
    }
  }

  float B[K][N];
  for (unsigned int i = 0; i < K; ++i) {
    for (unsigned int j = 0; j < N; ++j) {
      B[i][j] = initVal--;
    }
  }
  float C[M][N];
  for (unsigned int i = 0; i < M; ++i) {
    for (unsigned int j = 0; j < N; ++j) {
      C[i][j] = 0;
    }
  }
  float Ref[M][N];
  for (unsigned int i = 0; i < M; ++i) {
    for (unsigned int j = 0; j < N; ++j) {
      Ref[i][j] = 0;
      for (unsigned int k = 0; k < K; ++k) {
        Ref[i][j] += A[i][k] * B[k][j];
      }
    }
  }

  cl_mem buf_0 = clCreateBuffer(ctx_0, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                M*K*4, A, NULL);
  cl_mem buf_1 = clCreateBuffer(ctx_0, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                N*K*4, B, NULL);
  cl_mem buf_2 = clCreateBuffer(ctx_0, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                M*N*4, C, NULL);

  errNum = clSetKernelArg(kernel_0, 0, 4, &M);
  errNum |= clSetKernelArg(kernel_0, 1, 4, &N);
  errNum |= clSetKernelArg(kernel_0, 2, 4, &K);

  void *ptr_6[1];
  ptr_6[0] = &buf_0;
  errNum |= clSetKernelArg(kernel_0, 3, 8, &buf_0);

  void *ptr_7[1];
  ptr_7[0] = &buf_1;
  errNum |= clSetKernelArg(kernel_0, 4, 8, &buf_1);

  void *ptr_8[1];
  ptr_8[0] = &buf_2;
  errNum |= clSetKernelArg(kernel_0, 5, 8, &buf_2);

  size_t gSize_0[2] = {M, N}; //  matrix size == 64*64
  size_t lSize_0[2] = {0x8, 0x8};   //  local size == 8*8
  errNum = clEnqueueNDRangeKernel(cmdQue_0, kernel_0, 2, NULL, gSize_0, lSize_0,
                                  0, NULL, NULL);

  // ptr_9=byte[324](0)
  int ptr_9[4096];
  clEnqueueReadBuffer(cmdQue_0, buf_2, 1, 0, 0x4000, ptr_9, 0, NULL, NULL);
  // rtCompareDataWithFile(ptr_9, GL_RED, GL_BYTE, 324, -1, "cmp_0.bin")
  std::cout << "begin to compare: " << std::endl;

  writeDataToFile<float>(Ref, "ref.txt", 64*64*4, 64);
  writeDataToFile<float>(ptr_9, "ptr.txt", 64 * 64 * 4, 64);
  clReleaseMemObject(buf_0);
  clReleaseMemObject(buf_1);
  clReleaseMemObject(buf_2);
  clReleaseCommandQueue(cmdQue_0);
  clReleaseKernel(kernel_0);
  clReleaseProgram(clProg_0);
  clReleaseContext(ctx_0);
}