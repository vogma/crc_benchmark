cmake_minimum_required(VERSION 3.31.6)
project(crc_benchmark C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Do not build shared libraries")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable Benchmark's tests")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable Benchmark's GTest tests")
add_subdirectory(submodules/google-benchmark)

add_executable(crc_benchmark
    src/crc_sse.c
    src/avx512_vpclmulqdq_v3s2x4e.c
    src/crc_gnulib.c
    src/avx512_vpclmulqdq_crc32c_v3s2x4.c
    src/crc_avx512_vpclmulqdq.c
    crc_benchmark.cpp
)

target_include_directories(crc_benchmark PRIVATE src)
target_link_libraries(crc_benchmark PRIVATE benchmark::benchmark pthread)