cmake_minimum_required(VERSION 3.20)
project(gpac_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(gpac_backend
    src/assembler.cpp
    src/verifier.cpp
    src/llvm_backend.cpp
    src/native_driver.cpp
)

target_include_directories(gpac_backend PUBLIC include runtime)

add_executable(gpac_backend_driver src/main.cpp)
target_link_libraries(gpac_backend_driver PRIVATE gpac_backend)
