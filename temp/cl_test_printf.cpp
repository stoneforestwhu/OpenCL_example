#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fstream>
#include <assert.h>
#include <CL/cl.h>

#pragma warning( disable:4996 )

using std::cout;
using std::endl;

/** convert the kernel file into a string */
int convertToString(const char* filename, std::string& s)
{
	size_t size;
	char* str;
	std::fstream f(filename, (std::fstream::in | std::fstream::binary));

	if (f.is_open())
	{
		size_t fileSize;
		f.seekg(0, std::fstream::end);
		size = fileSize = (size_t)f.tellg();
		f.seekg(0, std::fstream::beg);
		str = new char[size + 1];
		if (!str)
		{
			f.close();
			return 0;
		}

		f.read(str, fileSize);
		f.close();
		str[size] = '\0';
		s = str;
		delete[] str;
		return 0;
	}
	cout << "Error: failed to open file\n:" << filename << endl;
	return -1;
}

int main() {
	printf("hello OpenCL\n");
	cl_int status = 0;
	size_t deviceListSize;

	char platform_info[1024];
	cl_platform_id* platform_list = (cl_platform_id*)malloc(sizeof(cl_platform_id));
	//  maybe not the gpu platform
	status = clGetPlatformIDs(1, platform_list, NULL);

	if (status != CL_SUCCESS) {
		printf("ERROR: Getting Platforms.(clGetPlatformIDs)\n");
		return EXIT_FAILURE;
	}
	clGetPlatformInfo(platform_list[0], CL_PLATFORM_NAME, 1024, platform_info, NULL);
	printf("\nPlatfom:\n\tName\t\t%s", platform_info);
	clGetPlatformInfo(platform_list[0], CL_PLATFORM_VERSION, 1024, platform_info, NULL);
	printf("\n\tVersion\t\t%s", platform_info);
	clGetPlatformInfo(platform_list[0], CL_PLATFORM_EXTENSIONS, 1024, platform_info, NULL);
	printf("\n\tExtension\t%s", platform_info);

	cl_device_id device;
	//创建 GPU 设备
	clGetDeviceIDs(platform_list[0], CL_DEVICE_TYPE_GPU,
		1, &device, NULL);

	//创建context
	cl_context context = clCreateContext(NULL,
		1, &device,
		NULL, NULL, NULL);

	// 装载内核程序，编译CL program ,生成CL内核实例
	std::string kernel_src;
	convertToString("kernels/cl_test_printf.cl", kernel_src);
	const char* source = kernel_src.c_str();
	size_t sourceSize[] = { kernel_src.length() };
	cl_program program = clCreateProgramWithSource(context,
		1,
		&source,
		sourceSize,
		&status);
	if (status != CL_SUCCESS) {
		printf("Error: Loading Binary into cl_program (clCreateProgramWithBinary)\n");
		assert(0);
	}

	// 为指定的设备编译CL program.
	status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	if (status != CL_SUCCESS) {
		char build_log[10240] = "\0";
		status = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 10240, build_log, NULL);
		printf("\n");
		printf("%s", build_log);
		printf("Error: Building Program (clBuildingProgram)\n");
		assert(0);
	}

	// 得到指定名字的内核实例的句柄
	cl_kernel kernel = clCreateKernel(program, "cl_test_printf", &status);
	if (status != CL_SUCCESS) {

		printf("Error: Creating Kernel from program.(clCreateKernel)\n");
		assert(0);
	}

	// 创建 OpenCL buffer 对象
	float* input_buffer = new float[144];
	float* output_buffer = new float[144];
	memset(input_buffer, 0, 144 * 4);
	memset(output_buffer, 0, 144 * 4);

	cl_mem inputBuffer = clCreateBuffer(
		context,
		CL_MEM_ALLOC_HOST_PTR,
		144 * 4,
		NULL,
		&status);

	if (status != CL_SUCCESS) {
		printf("Error: Create Buffer, inputbuffer. (clCreateBuffer)\n");
		assert(0);
	}

	cl_mem outputBuffer = clCreateBuffer(
		context,
		CL_MEM_ALLOC_HOST_PTR,
		144 * 4,
		NULL,
		&status);

	if (status != CL_SUCCESS) {
		printf("Error: Create Buffer, outputBuffer. (clCreateBuffer)\n");
		return EXIT_FAILURE;
	}


	//  为内核程序设置参数
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&inputBuffer);
	if (status != CL_SUCCESS) {
		printf("Error: Setting kernel argument. (clSetKernelArg)\n");
		assert(0);
	}

	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&outputBuffer);
	if (status != CL_SUCCESS) {
		printf("Error: Setting kernel argument. (clSetKernelArg)\n");
		return EXIT_FAILURE;
	}

	// 创建一个OpenCL command queue
	cl_command_queue commandQueue = clCreateCommandQueue(context, \
		device, \
		0, \
		&status);
	if (status != CL_SUCCESS) {
		printf("Error: Create Command Queue. (clCreateCommandQueue)\n");
		return EXIT_FAILURE;
	}


	// 将一个kernel 放入 command queue
	size_t globalThreads[] = { 4, 4 };
	size_t localThreads[] = { 3, 3 };
	status = clEnqueueNDRangeKernel(commandQueue, kernel, \
		2, NULL, globalThreads, \
		localThreads, 0, \
		NULL, NULL);
	if (status != CL_SUCCESS) {
		printf("Error: Enqueueing kernel\n");
		return EXIT_FAILURE;
	}

	// 确认 command queue 中所有命令都执行完毕
	status = clFinish(commandQueue);
	if (status != CL_SUCCESS) {
		printf("Error: Finish command queue\n");
		return EXIT_FAILURE;
	}

	// 将内存对象中的结果读回Host
	status = clEnqueueReadBuffer(commandQueue,
		outputBuffer, CL_TRUE, 0,
		4 * 4, output_buffer, 0, NULL, NULL);
	if (status != CL_SUCCESS) {
		printf("Error: Read buffer queue\n");
		return EXIT_FAILURE;
	}

	// Host端打印结果
	printf("out:\n");
	for (int i = 0; i < 4; ++i) {
		printf("%f \n", output_buffer[i]);
	}

	// 资源回收
	status = clReleaseKernel(kernel);
	status = clReleaseProgram(program);
	status = clReleaseMemObject(outputBuffer);
	status = clReleaseCommandQueue(commandQueue);
	status = clReleaseContext(context);

	//free(device);
	delete input_buffer;
	delete output_buffer;

	return 0;

}