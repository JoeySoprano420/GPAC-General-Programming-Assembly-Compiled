cmake_minimum_required(VERSION 3.20)
project(gpac_pipeline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(gpac_zvm runtime/gpac_zvm_runtime.cpp)
target_include_directories(gpac_zvm PUBLIC runtime include)

add_executable(gpac_frontend src/main.cpp src/frontend.cpp src/llvm_lowering.cpp)
target_include_directories(gpac_frontend PRIVATE include runtime)
target_link_libraries(gpac_frontend PRIVATE gpac_zvm)
