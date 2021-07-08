# cmake minimum version required
cmake_minimum_required (VERSION 2.8)
# project information

project (TestGemm)

# CUDA home
set(CUDA_HOME "C:/Program\ Files/NVIDIA\ GPU\ Computing\ Toolkit/CUDA/v11.1")

include_directories(${CUDA_HOME}/include)
include_directories(inc)
link_directories(${CUDA_HOME}/lib/x64)

add_executable(test_gemm1 main1.cpp)
target_link_libraries(test_gemm1 OpenCL)

add_executable(test_gemm2 main2.cpp)
target_link_libraries(test_gemm2 OpenCL)

add_executable(test_gemm3 main3.cpp)
target_link_libraries(test_gemm3 OpenCL)

add_executable(test_gemm4 main4.cpp)
target_link_libraries(test_gemm4 OpenCL)
