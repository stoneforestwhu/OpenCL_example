#include <assert.h>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <io.h>
#include<CL/cl.h>

#define ARRAY_SIZE 10

using std::ifstream;
using std::ios;
using std::ostringstream;
using std::string;
using std::vector;
using std::shared_ptr;

std::shared_ptr<char> readKernelFile(std::string strSource) {
  std::ifstream kernelfile(strSource);
  if (kernelfile.fail()) {
    std::cout << "Can not open it " << std::endl;
    // throw new std::runtime_error("IO stream corrupted");
  }
  std::string shaderStr((std::istreambuf_iterator<char>(kernelfile)),
                        std::istreambuf_iterator<char>());
  kernelfile.close();
  size_t len = shaderStr.length();
  std::shared_ptr<char> shaderPtr(new char[len + 1]);
  strcpy_s(shaderPtr.get(), len + 1, shaderStr.c_str());
  std::cout << shaderStr << std::endl;
  return shaderPtr;
} 

cl_context CreateContext(cl_device_id *device)
{
	cl_int errNum;
	cl_uint numPlatforms;
	cl_platform_id firstPlatformId;
	cl_context context = NULL;
	errNum = clGetPlatformIDs(1, &firstPlatformId, &numPlatforms);
	if (errNum != CL_SUCCESS || numPlatforms <= 0)
	{
		printf("Failed to find any OpenCL platforms.");
		return NULL;
	}
	errNum = clGetDeviceIDs(firstPlatformId, CL_DEVICE_TYPE_GPU, 1, device, NULL);
	if (errNum != CL_SUCCESS)
	{
		printf("There is no GPU, trying CPU ... ");
		errNum = clGetDeviceIDs(firstPlatformId, CL_DEVICE_TYPE_CPU, 1, device, NULL);
	}

	if (errNum != CL_SUCCESS)
	{
		printf("There is NO GPU or CPU");
		return NULL;
	}

	context = clCreateContext(NULL, 1, device, NULL, NULL, &errNum);
	if (errNum != CL_SUCCESS)
	{
		printf("create context error\n");
		return NULL;
	}
	return context;
}

cl_command_queue CreateCommandQueue(cl_context context, cl_device_id device)
{
	cl_command_queue commandQueue = NULL;
	commandQueue = clCreateCommandQueue(context, device, 0, NULL);
	if (commandQueue == NULL)
	{
		printf("Failed to create commandQueue for device 0");
		return NULL;
	}
	return commandQueue;
}

cl_program CreateProgram(cl_context context, cl_device_id device, const char *fileName)
{
	cl_int errNum;
	cl_program program;
	//char *const source = ReadKernelSourceFile(fileName, &program_length);
  shared_ptr<char> src_ptr = readKernelFile(fileName);
  char *const source = src_ptr.get();
	program = clCreateProgramWithSource(context, 1, (const char **)&source, NULL, NULL);

	if (program == NULL)
	{
		printf("Failed to create CL program from source");
		return NULL;
	}
	errNum = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		char buildLog[16384];
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
		printf("Error in kernel:%s ", buildLog);
		clReleaseProgram(program);
		return NULL;
	}
	return program;
}

bool CreateMemObjects(cl_context context, cl_mem memObjects[3], float* a, unsigned int size_a, float* b, unsigned int size_b, 
	                    unsigned int size_c)
{
  memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(float) * size_a, a, NULL);
  memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(float) * size_b, b, NULL);
  memObjects[2] = clCreateBuffer(context, CL_MEM_READ_WRITE,
                                 sizeof(float) * size_c, NULL, NULL);
	if (memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL)
	{
		printf("Error creating memory objects.");
		return false;
	}
	return true;
}

void initMatrix(float *matrix, unsigned int m, unsigned int n) { 
	for (unsigned int i = 0; i < m; i++) {
    for (unsigned int j = 0; j < n; j++) {
      matrix[i*n + j] = i+j;
      std::cout << matrix[i * n + j] << " ";
		}
    std::cout << "\n";
	}
}

void Cleanup(cl_context context, cl_command_queue commandQueue, cl_program program, cl_kernel kernel, cl_mem memObjects[3])
{
	for (int i = 0; i < 3; ++i)
	{
		if (memObjects[i] != 0)
			clReleaseMemObject(memObjects[i]);
	}

	if (commandQueue != 0)
		clReleaseCommandQueue(commandQueue);
	if (kernel != 0)
		clReleaseKernel(kernel);
	if (program != 0)
		clReleaseProgram(program);
	if (context != 0)
		clReleaseContext(context);
}

int main(int argc, char** argv)
{
	cl_context context = 0;
	cl_command_queue commandQueue = 0;
	cl_program program = 0;
	cl_device_id device = 0;
	cl_kernel kernel = 0;
	cl_mem memObjects[3] = { 0, 0, 0 };
	cl_int errNum;

	context = CreateContext(&device);
	if (context == NULL)
	{
		printf("Failed to create OpenCL context.");
		return 1;
	}
	commandQueue = CreateCommandQueue(context, device);
	if (commandQueue == NULL)
	{
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}

	program = CreateProgram(context, device, "kernel/matrix_multiply.cl");
	if (program == NULL)
	{
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}

	kernel = clCreateKernel(program, "matrix_multiply", NULL);
	if (kernel == NULL)
	{
		printf("Failed to create kernel");
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}

	int M = 9;
	int C = 9;
	int N = 9;
	shared_ptr<float> matrixA(new float[M*C]);
  shared_ptr<float> matrixB(new float[C*N]);
  shared_ptr<float> matrixC(new float[M*N]);

	std::cout << "initial matrixA:" << std::endl;
	initMatrix(matrixA.get(), M, C);
  std::cout << "initial matrixB:" << std::endl;
  initMatrix(matrixB.get(), C, N);
  std::cout << "initial matrixC:" << std::endl;
  initMatrix(matrixC.get(), M, N);

	cl_mem matrixMemObj[3];
  memObjects[0] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     sizeof(float) * M * C, matrixA.get(), NULL);
  memObjects[1] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                           sizeof(float) * C * N, matrixB.get(), NULL);
  memObjects[2] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     sizeof(float) * M * N, matrixC.get(), NULL);

	if (memObjects[0] == NULL || memObjects[1] == NULL || memObjects[2] == NULL) {
    printf("Error creating memory objects.");
    Cleanup(context, commandQueue, program, kernel, memObjects);
    return false;
  }

	//  clSetKernelArg()
	errNum = clSetKernelArg(kernel, 0, sizeof(int), &M);
  errNum |= clSetKernelArg(kernel, 1, sizeof(int), &N);
  errNum |= clSetKernelArg(kernel, 2, sizeof(int), &C);
  errNum |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &memObjects[0]);
	errNum |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &memObjects[1]);
	errNum |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &memObjects[2]);
	if(errNum != CL_SUCCESS)
	{
		printf("Error setting kernel arguments.");
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}
	size_t globalWorkSize[2] = { 9, 9 };
	size_t localWorkSize[2] = { 3, 3 };
	errNum = clEnqueueNDRangeKernel(commandQueue,   
		                            kernel, 
		                            2,                //   work_dim 只能是1到3之间
		                            NULL,             //   global_work_offset
		                            globalWorkSize,   //   global_work_size
		                            localWorkSize,    //   local_work_size
		                            0,                //   cl_uint  num_events_in_wait_list
		                            NULL,             //   event_wait_list
		                            NULL);            //   event
	if (errNum != CL_SUCCESS)
	{
		printf("Error queuing kernel for execution.");
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}

	float result[9][9];
	errNum = clEnqueueReadBuffer(commandQueue, memObjects[2], CL_TRUE, 0, 81 * sizeof(float), result, 0, NULL, NULL);
	if (errNum != CL_SUCCESS)
	{
		printf("Error reading result buffer.");
		Cleanup(context, commandQueue, program, kernel, memObjects);
		return 1;
	}

	for (int i = 0; i < 9; ++i) {
    for (int j = 0; j < 9; ++j) {
      printf("%f ", result[i][j]);
    }
    printf("\n");
  }
  
	printf("\n");
	
	float refVal[9][9] = {0.0};
	float* ma = matrixA.get();
	float* mb = matrixB.get();
  for (int i = 0; i < 9; ++i) {
    for (int j = 0; j < 9; ++j) {
      for (int t = 0; t < 9; ++t)
        refVal[i][j] += (*(ma + i * 9 + t)) * (*(mb + t * 9 + j));
      printf("%f ", refVal[i][j]);
		}
    printf("\n");
  }


	printf("Executed program successfully.");
	Cleanup(context, commandQueue, program, kernel, memObjects);
	return 0;
}