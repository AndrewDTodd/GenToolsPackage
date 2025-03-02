# toolchain-Linux_gcc_x86.cmake

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86)

# Use GCC and G++
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# Enable SIMD extensions if supported
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32 -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32 -march=native")
