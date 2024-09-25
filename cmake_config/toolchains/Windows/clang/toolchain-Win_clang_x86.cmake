# toolchain-Win_clang_x86.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86)

# Use clang and clang++ for 32-bit
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_FLAGS "-m32")
set(CMAKE_CXX_FLAGS "-m32")