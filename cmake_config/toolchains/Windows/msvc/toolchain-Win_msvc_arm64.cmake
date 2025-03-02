# toolchain-Win_msvc_arm64.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR arm64)

set(CMAKE_C_COMPILER cl.exe)
set(CMAKE_CXX_COMPILER cl.exe)

set(CMAKE_C_FLAGS "/arch:ARM64")
set(CMAKE_CXX_FLAGS "/arch:ARM64 /EHsc /Zc:__cplusplus")