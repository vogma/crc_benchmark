cmake_minimum_required(VERSION 3.31.6)


set(TARGET_ARCH "x86" CACHE STRING "Target architecture to build: x86 or aarch64")
set_property(CACHE TARGET_ARCH PROPERTY STRINGS x86 aarch64)
message(STATUS "Selected target architecture: ${TARGET_ARCH}")

# If building for AArch64, set up cross-compilers BEFORE calling project()
if(TARGET_ARCH STREQUAL "aarch64")
    # Specify system name for cross-compiling
    set(CMAKE_SYSTEM_NAME Linux CACHE STRING "")

    if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "aarch64")
        message(STATUS "Host is aarch64. Using default compiler")
        add_compile_options(-march=native -O2)

    else()
        add_compile_options(-march=armv8.4-a+crc+crypto+sha3 -O2)

        set(CMAKE_C_COMPILER aarch64-none-linux-gnu-gcc CACHE PATH "C compiler for AArch64" FORCE)
        set(CMAKE_CXX_COMPILER aarch64-none-linux-gnu-g++ CACHE PATH "C++ compiler for AArch64" FORCE)
        message(STATUS "Using cross-compiler: ${CMAKE_C_COMPILER} / ${CMAKE_CXX_COMPILER}")
    endif()
elseif(TARGET_ARCH STREQUAL "x86")
    add_compile_options(-march=native -O2)
else()
    message(FATAL_ERROR "Unsupported TARGET_ARCH: ${TARGET_ARCH}. Use x86 or aarch64")
endif()

project(crc_benchmark VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Do not build shared libraries")
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable Benchmark's tests")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable Benchmark's GTest tests")
add_subdirectory(submodules/google-benchmark)

if(TARGET_ARCH STREQUAL "x86")
    add_compile_options(-march=native)
add_executable(crc_benchmark_x86
    src/x86/crc_sse.c
    src/x86/avx512_vpclmulqdq_v3s2x4e.c
    src/base/crc_gnulib.c
    src/x86/avx512_vpclmulqdq_crc32c_v3s2x4.c
    src/x86/crc_avx512_vpclmulqdq.c
    crc_x86_benchmark.cpp
)
target_include_directories(crc_benchmark_x86 PRIVATE src)
target_link_libraries(crc_benchmark_x86 PRIVATE benchmark::benchmark pthread)

else()
    add_executable(crc_benchmark_aarch64
    src/aarch64/crc32_chrome_scalar_opt.c
    src/aarch64/crc32_chromium_scalar.c
    src/aarch64/crc32_simd_cloudfare_zlib.c
    src/base/crc_gnulib.c
    src/aarch64/crc32_simd_chromium.c
    crc_aarch64_benchmark.cpp
)
target_include_directories(crc_benchmark_aarch64 PRIVATE src)
target_link_libraries(crc_benchmark_aarch64 PRIVATE benchmark::benchmark pthread)

endif()
