# CMakeList.txt : CMakeLists for CmdLineParser, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.20)

project(CmdLineParser)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

#add _DEBUG macro based on build type for code that should compile differently in Debug vs Release
if(CMAKE_BUILD_TYPE MATCHES "Debug")
	add_definitions(-D_DEBUG)
endif()

#Google tests require mimimum c++ version 14
#setting minimum c++ version to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(UNIX AND NOT APPLE)
	add_definitions(-DTARGET_OS="Linux")
	add_definitions(-D_LINUX_TARGET)
elseif(WIN32)
	add_definitions(-DTARGET_OS="Windows")
	add_definitions(-D_WIN_TARGET)
elseif(UNIX AND APPLE)
	add_definitions(-DTARGET_OS="MacOS")
	add_definitions(-D_MAC_TARGET)
else()
	add_definitions(-DTARGET_OS="Unkown")
endif()

add_definitions(
        -DRESET="\\033[0m"
        -DRED="\\033[31m"
        -DGREEN="\\033[32m"
        -DYELLOW="\\033[33m"
        -DBLUE="\\033[34m"
		-DDIMB="\\033[2m"
		-DNORM_TXT="\\03[22m"
    )

#A significant release that likely includes new features, changes, and enhancements that may not be backward compatible with the previous version
set(VERSION_MAJOR 1)
#A release that may add new features but remains backward compatible with the previous version
set(VERSION_MINOR 0)
#A release that includes minor changes, bug fixes, and patches without adding new features. It is typically backward and forward compatible with the corresponding minor version
set(VERSION_PATCH 0)
#A version that is often used to denote a specific build of the software, including revisions, builds, or other metadata
set(VERSION_BUILD 0)

#Add versioning definitions for the CmdLineParser library
add_definitions(-DVERSION_MAJOR=${VERSION_MAJOR})
add_definitions(-DVERSION_MINOR=${VERSION_MINOR})
add_definitions(-DVERSION_PATCH=${VERSION_PATCH})
add_definitions(-DVERSION_BUILD=${VERSION_BUILD})

set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)
configure_file(rootConfig.h.in ${GENERATED_DIR}/rootConfig.h)

include_directories(${GENERATED_DIR})

set(CmdLineParser_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCE_FILES "${CmdLineParser_SOURCE_DIR}/src/*.cpp" "${CmdLineParser_SOURCE_DIR}/generated/*.cpp")

add_library(CmdLineParser STATIC ${SOURCE_FILES})
#include the header files in the include directory
target_include_directories(CmdLineParser PUBLIC ${CmdLineParser_SOURCE_DIR}/include)
target_include_directories(CmdLineParser PUBLIC ${CmdLineParser_SOURCE_DIR}/include/FlagDerivedTypes)

#install the public header files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ 
DESTINATION CmdLineParser/include 
FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

#install the static library
install(TARGETS CmdLineParser DESTINATION CmdLineParser/lib)

enable_testing()
add_subdirectory(tests)