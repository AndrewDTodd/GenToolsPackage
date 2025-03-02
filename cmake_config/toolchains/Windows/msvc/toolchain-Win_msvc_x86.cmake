# toolchain-Win_msvc_x86.cmake
set(CMAKE_C_COMPILER cl.exe)
set(CMAKE_CXX_COMPILER cl.exe)

set(CMAKE_C_FLAGS "/arch:IA32")
set(CMAKE_CXX_FLAGS "/arch:IA32 /EHsc /Zc:__cplusplus")