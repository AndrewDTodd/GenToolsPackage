# toolchain-Linux_clang_arm64.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Use the Clang cross-compiler for arm64
Set(CMAKE_C_COMPILER aarch64-linux-gnu-clang)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-clang++)