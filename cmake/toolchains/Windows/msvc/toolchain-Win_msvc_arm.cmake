# toolchain-Win_msvc_arm.cmake
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER cl)
set(CMAKE_CXX_COMPILER cl)

set(CMAKE_C_FLAGS "/arch:ARM")
set(CMAKE_CXX_FALGS "/arch:ARM")