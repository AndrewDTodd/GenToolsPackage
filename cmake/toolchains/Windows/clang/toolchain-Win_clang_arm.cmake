# toolchain-Win_clang_arm.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Use the clang cross-compiler for arm
set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)

set(CMAKE_C_FLAGS "--target=armv7-windows-msvc")
set(CMAKE_CXX_FLAGS "--target=armv7-windows-msvc")