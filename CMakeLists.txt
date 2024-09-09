# CMakeList.txt : CMakeLists for TokenValueParser, include source and define
# project specific logic here.

cmake_minimum_required (VERSION 3.20)

project(TokenValueParser)

#include(DetectArchitecture.cmake)

#message(STATUS "Detected System Architecture: ${SYSTEM_ARCHITECTURE}")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# add _DEBUG macro based on build type for code that should compile differently in Debug vs Release
if(CMAKE_BUILD_TYPE MATCHES "Debug")
	add_definitions(-D_DEBUG)
endif()

# Google tests require mimimum c++ version 14
# setting minimum c++ version to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect OS for platform specific compilation
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

# Add definitions (become c++ pre-processor definitions) for ANCI character codes for rich text in terminal
add_definitions(
    -DRESET="\\033[0m"
    -DRED="\\033[31m"
    -DGREEN="\\033[32m"
    -DYELLOW="\\033[33m"
    -DBLUE="\\033[34m"
	-DDIMB="\\033[2m"
	-DNORM_TXT="\\03[22m"
)

# A significant release that likely includes new features, changes, and enhancements that may not be backward compatible with the previous version
set(VERSION_MAJOR 1)
# A release that may add new features but remains backward compatible with the previous version
set(VERSION_MINOR 0)
# A release that includes minor changes, bug fixes, and patches without adding new features. It is typically backward and forward compatible with the corresponding minor version
set(VERSION_PATCH 0)
# A version that is often used to denote a specific build of the software, including revisions, builds, or other metadata
set(VERSION_BUILD 0)

# Add versioning definitions for the TokenValueParser library
add_definitions(-DVERSION_MAJOR=${VERSION_MAJOR})
add_definitions(-DVERSION_MINOR=${VERSION_MINOR})
add_definitions(-DVERSION_PATCH=${VERSION_PATCH})
add_definitions(-DVERSION_BUILD=${VERSION_BUILD})

# Create a CMake option variable used to trigger settings for the special Release-Dev build mode of this project
option(RELEASE_ERROR_MSG "Include release error messages in the build" OFF)

# Define custom configuration types for Debug, Release, and Release-Dev
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Release-Dev" CACHE STRING "List of supported build modes." FORCE)

# Set compiler and linker flags for the Release-Dev build mode
if(CMAKE_BUILD_TYPE STREQUAL "Release-Dev")
	# Set RELEASE_ERROR_MSG to 1 in the Release-Dev mode
	add_compile_definitions(RELEASE_ERROR_MSG=1)
else()
	# Set RELEASE_ERROR_MSG to 0 in other modes if not explicitly set
	add_compile_definitions(RELEASE_ERROR_MSG=0)
endif()

# ***************************************************************************************************************************************************
# Begin Target Configuration

# Create CMake referecne to a directory for configuration templates
set(CONFIG_TEMPLATES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config_templates)

# Create CMake reference to a directory for CMake generated files
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)
# Create CMake reference to generated directory sub-directories for headers and source files
set(GENERATED_INCLUDE_DIR ${GENERATED_DIR}/include)
set(GENERATED_SRC_DIR ${GENERATED_DIR}/src)

# Create the generated directory and sub-directory structure if it doesn't exist
file(MAKE_DIRECTORY ${GENERATED_INCLUDE_DIR})
file(MAKE_DIRECTORY ${GENERATED_SRC_DIR})

# Function to process the configuration templates and generate source
function(generate_from_template TEMPLATE_DIR OUTPUT_DIR FILE_TYPE)
	file(GLOB TEMPLATES "${TEMPLATE_DIR}/*.in")
	foreach(TEMPLATE ${TEMPLATES})
		get_filename_component(FILENAME ${TEMPLATE} NAME_WE)
		set(OUTPUT_FILE ${OUTPUT_DIR}/${FILENAME}.${FILE_TYPE})
		configure_file(${TEMPLATE} ${OUTPUT_FILE} @ONLY)
	endforeach()
endfunction()

# Generate headers and source files from templates
generate_from_template(${CONFIG_TEMPLATES_DIR}/include ${GENERATED_INCLUDE_DIR} h)
generate_from_template(${CONFIG_TEMPLATES_DIR}/src ${GENERATED_SRC_DIR} cpp)

# Create CMake reference to this source directory, used in sub-directory CMake files
set(TokenValueParser_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Create list of all source files to be used in this target
file(GLOB_RECURSE SOURCE_FILES "${TokenValueParser_SOURCE_DIR}/src/*.cpp" "${GENERATED_SRC_DIR}/*.cpp")

add_library(TokenValueParser STATIC ${SOURCE_FILES})
# Include the project header files in the include directory
target_include_directories(TokenValueParser PUBLIC ${TokenValueParser_SOURCE_DIR}/include)
target_include_directories(TokenValueParser PUBLIC ${TokenValueParser_SOURCE_DIR}/include/FlagDerivedTypes)

# Include the generated headers in the generated directory
target_include_directories(TokenValueParser PUBLIC ${GENERATED_INCLUDE_DIR})

# Set pre-processor definition to be used in code to build release time error messaging when activated
if(NOT RELEASE_ERROR_MSG)
	target_compile_definitions(TokenValueParser PUBLIC RELEASE_ERROR_MSG=0)
else()
	target_compile_definitions(TokenValueParser PUBLIC RELEASE_ERROR_MSG=1)
endif()

#Install the public header files/include public headers in library package
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ 
DESTINATION TokenValueParser/include 
FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

# Install the static library/include the library binaries in the library package
install(TARGETS TokenValueParser DESTINATION TokenValueParser/lib)

# Configure CMake for testing. Needed for GTest features
enable_testing()

# Connect the tests subdirectory and its targets to the root target
add_subdirectory(tests)