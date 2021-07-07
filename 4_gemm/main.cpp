#include "util.h"

#define uint unsigned int


int main(){
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
      readKernelFile("kernel/kernel3.txt");
  char *clProg_0_src_0 = clProg_0_src_0_shared_ptr.get();
  char *clProg_0_srcPtr_0[1] = {clProg_0_src_0};
  cl_program clProg_0 = clCreateProgramWithSource(
      ctx_0, 1, (const char **)clProg_0_srcPtr_0, NULL, NULL);
  clBuildProgram(clProg_0, 0, NULL, NULL, NULL, NULL);
  cl_kernel kernel_0 = clCreateKernel(clProg_0, "myGEMM3", NULL);

  float A[64][64];
  int initVal = 0;
  for (unsigned int i = 0; i < 64; ++i) {
    for (unsigned int j = 0; j < 64; ++j) {
      A[i][j] = initVal++; 
    }
  }

  float B[64][64];
  for (unsigned int i = 0; i < 64; ++i) {
    for (unsigned int j = 0; j < 64; ++j) {
      B[i][j] = initVal--;
    }
  }
  float C[64][64];
  for (unsigned int i = 0; i < 64; ++i) {
    for (unsigned int j = 0; j < 64; ++j) {
      B[i][j] = 0;
    }
  }
  float Ref[64][64];
  for (unsigned int i = 0; i < 64; ++i) {
    for (unsigned int j = 0; j < 64; ++j) {
      Ref[i][j] = 0;
      for (unsigned int k = 0; k < 64; ++k) {
        Ref[i][j] += A[i][k] * B[k][j];
      }
    }
  }


  cl_mem buf_0 = clCreateBuffer(ctx_0, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                0x4000, A, NULL);
  cl_mem buf_1 = clCreateBuffer(ctx_0, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                0x4000, B, NULL);
  cl_mem buf_2 = clCreateBuffer(ctx_0, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                0x4000, C, NULL);

  uint ptr_3[1] = {0x40};
  errNum = clSetKernelArg(kernel_0, 0, 4, ptr_3);

  uint ptr_4[1] = {0x40};
  errNum |= clSetKernelArg(kernel_0, 1, 4, ptr_4);

  uint ptr_5[1] = {0x40};
  errNum |= clSetKernelArg(kernel_0, 2, 4, ptr_5);

  void *ptr_6[1];
  ptr_6[0] = &buf_0;
  errNum |= clSetKernelArg(kernel_0, 3, 8, ptr_6[0]);

  void *ptr_7[1];
  ptr_7[0] = &buf_1;
  errNum |= clSetKernelArg(kernel_0, 4, 8, ptr_7[0]);

  void *ptr_8[1];
  ptr_8[0] = &buf_2;
  errNum |= clSetKernelArg(kernel_0, 5, 8, ptr_8[0]);

  size_t gSize_0[2] = {0x40, 0x10};  //  matrix size == 64*64
  size_t lSize_0[2] = {0x8, 0x2};    //  local size == 8*8
  errNum = clEnqueueNDRangeKernel(cmdQue_0, kernel_0, 2, NULL, gSize_0, lSize_0,
                                  0, NULL, NULL);

  // ptr_9=byte[324](0)
  int ptr_9[4096];
  clEnqueueReadBuffer(cmdQue_0, buf_2, 1, 0, 0x4000, ptr_9, 0, NULL, NULL);
  // rtCompareDataWithFile(ptr_9, GL_RED, GL_BYTE, 324, -1, "cmp_0.bin")
  std::cout << "begin to compare: " << std::endl;
  for (unsigned int i = 0; i < 64; ++i) {
    for (unsigned int j = 0; j < 64; ++j) {
      if (ptr_9[i * 64 + j] != Ref[i][j])
        std::cout << "Ref[" << i << "][" << j << "] != ptr_9[" << i << "][" << j << "]" << std::endl;
    }
  }

  clReleaseMemObject(buf_0);
  clReleaseMemObject(buf_1);
  clReleaseMemObject(buf_2);
  clReleaseCommandQueue(cmdQue_0);
  clReleaseKernel(kernel_0);
  clReleaseProgram(clProg_0);
  clReleaseContext(ctx_0);
}