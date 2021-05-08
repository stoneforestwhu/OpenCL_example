# cmake minimum version required
cmake_minimum_required (VERSION 2.8)
# project information

project (MatrixMultiply)

# CUDA home
set(CUDA_HOME "C:/Program\ Files/NVIDIA\ GPU\ Computing\ Toolkit/CUDA/v11.1")

include_directories(${CUDA_HOME}/include)
link_directories(${CUDA_HOME}/lib/x64)

add_executable(matrix_multiply src/matrix_multiply.cpp)
target_link_libraries(matrix_multiply OpenCL)
