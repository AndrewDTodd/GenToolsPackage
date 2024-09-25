# toolchain-Win_clang_arm64.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Use the Clang cross-compiler for arm64
set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)

set(CMAKE_C_FLAGS "--target=aarch64-windows-msvc")
set(CMAKE_CXX_FLAGS "--target=aarch64-windows-msvc")