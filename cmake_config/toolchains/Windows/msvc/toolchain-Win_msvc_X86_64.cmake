# toolchain-Win_msvc_X86_64.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER cl.exe)
set(CMAKE_CXX_COMPILER cl.exe)

set(CMAKE_C_FLAGS "/arch:AVX")
set(CMAKE_CXX_FLAGS "/arch:AVX /EHsc /Zc:__cplusplus")