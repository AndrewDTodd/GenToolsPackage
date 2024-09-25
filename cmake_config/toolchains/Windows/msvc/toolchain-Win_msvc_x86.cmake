# toolchain-Win_msvc_x86.cmake
set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

set(CMAKE_C_FLAGS "/arch:IA32")
set(CMAKE_CXX_FLAGS "/arch:IA32")