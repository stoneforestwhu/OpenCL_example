#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <CL/cl.h>

int main()
{
	char info[1024];

	cl_uint nPlatform;
	cl_platform_id *listPlatform;
	cl_uint nDevice;
	cl_device_id *listDevice;
	//  first step, query how many platforms we have in current machine.
	clGetPlatformIDs(0, NULL, &nPlatform);
	//  new a list which will contain all the platform_id of all the platforms.
	listPlatform = (cl_platform_id*)malloc(nPlatform * sizeof(cl_platform_id));
	//  second step, get all platform ids
	clGetPlatformIDs(nPlatform, listPlatform, NULL);

	for (unsigned int i = 0; i < nPlatform; i++) {
		clGetPlatformInfo(listPlatform[i], CL_PLATFORM_NAME, 1024, info, NULL);
		printf("\nPlatfom[%d]:\n\tName\t\t%s", i, info);
		clGetPlatformInfo(listPlatform[i], CL_PLATFORM_VERSION, 1024, info, NULL);
		printf("\n\tVersion\t\t%s", info);
		clGetPlatformInfo(listPlatform[i], CL_PLATFORM_EXTENSIONS, 1024, info, NULL);
		printf("\n\tExtension\t%s", info);

		//  each platforms may not contain only 1 devices.
		clGetDeviceIDs(listPlatform[i], CL_DEVICE_TYPE_ALL, 0, NULL, &nDevice);
		listDevice = (cl_device_id*)malloc(nDevice * sizeof(cl_device_id));
		clGetDeviceIDs(listPlatform[i], CL_DEVICE_TYPE_ALL, nDevice, listDevice, NULL);

		for (unsigned int j = 0; j < nDevice; j++) {
			printf("\n");
			clGetDeviceInfo(listDevice[j], CL_DEVICE_NAME, 1024, info, NULL);
			printf("\n\tDevice[%d]:\n\tName\t\t%s", j, info);
			clGetDeviceInfo(listDevice[j], CL_DEVICE_VERSION, 1024, info, NULL);
			printf("\n\tVersion\t\t%s", info);
			clGetDeviceInfo(listDevice[j], CL_DEVICE_TYPE, 1024, info, NULL);
			switch (info[0])
			{
				case CL_DEVICE_TYPE_DEFAULT:strcpy_s(info, "DEFAULT"); break;
				case CL_DEVICE_TYPE_CPU:strcpy_s(info, "CPU"); break;
				case CL_DEVICE_TYPE_GPU:strcpy_s(info, "GPU"); break;
				case CL_DEVICE_TYPE_ACCELERATOR:strcpy_s(info, "ACCELERATOR"); break;
				case CL_DEVICE_TYPE_CUSTOM:strcpy_s(info, "CUSTOM"); break;
				case CL_DEVICE_TYPE_ALL:strcpy_s(info, "ALL"); break;
			}
			printf("\n\tType\t\t%s\n", info);

			/*
			cl_device_svm_capabilities svm;
			clGetDeviceInfo(listDevice[j], CL_DEVICE_VERSION, sizeof(cl_device_svm_capabilities), &svm, NULL);
			info[0] = '\0';
			if (svm & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER)
			strcat_s(info, "COARSE_GRAIN_BUFFER ");
			if (svm & CL_DEVICE_SVM_FINE_GRAIN_BUFFER)
			strcat_s(info, "FINE_GRAIN_BUFFER ");
			if (svm & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM)
			strcat_s(info, "FINE_GRAIN_SYSTEM ");
			if (svm & CL_DEVICE_SVM_ATOMICS)
			strcat_s(info, "ATOMICS");
			printf("\n\tSVM\t\t%s", info);
			*/
		}

		printf("\n\n");
		free(listDevice);
	}
	free(listPlatform);
	return 0;
}