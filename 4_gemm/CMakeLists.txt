# cmake minimum version required
cmake_minimum_required (VERSION 2.8)
# project information

project (TestGemm)

# CUDA home
set(CUDA_HOME "C:/Program\ Files/NVIDIA\ GPU\ Computing\ Toolkit/CUDA/v11.1")

include_directories(${CUDA_HOME}/include)
include_directories(inc)
link_directories(${CUDA_HOME}/lib/x64)

add_executable(test_gemm main.cpp)
target_link_libraries(test_gemm OpenCL)
