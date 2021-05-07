#pragma OPENCL EXTENSION cl_khr_fp16 : enable
__kernel void cl_test_printf(__global half *input_buffer, __global float *output_buffer)
{
	size_t idx = get_global_id(0);
	//half tmp_1 = 1.2f;
	//half tmp_2 = 1.3f;
	input_buffer[idx] = 1.2f;
	output_buffer[idx] = vload_half(idx, input_buffer);
	printf("global_id: %d\n", idx);
}