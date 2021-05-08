# cmake minimum version required
cmake_minimum_required (VERSION 2.8)
# project information

project (VectorAdd)

# CUDA home
set(CUDA_HOME "C:/Program\ Files/NVIDIA\ GPU\ Computing\ Toolkit/CUDA/v11.1")

include_directories(${CUDA_HOME}/include)
link_directories(${CUDA_HOME}/lib/x64)

add_executable(vector_add src/vec_add.cpp)
target_link_libraries(vector_add OpenCL)