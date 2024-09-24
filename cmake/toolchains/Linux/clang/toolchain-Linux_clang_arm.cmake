# toolchain-Linux_clang_arm.cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Use the clang cross-compiler for arm
set(CMAKE_C_COMPILER arm-linux-gnueabihf-clang)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-clang++)