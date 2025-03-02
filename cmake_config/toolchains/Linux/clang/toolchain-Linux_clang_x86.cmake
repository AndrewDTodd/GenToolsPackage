# toolchain-Linux_clang_x86.cmake

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86)

# Use clang and clang++
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Enable SIMD extensions if supported
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -march=native")
