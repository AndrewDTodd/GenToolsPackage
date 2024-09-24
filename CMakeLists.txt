# CMakeList.txt : CMakeLists for TokenValueParser, include source and define
# project specific logic here.

cmake_minimum_required (VERSION 3.20)

project(GenToolsPackage)

#include(DetectArchitecture.cmake)

#message(STATUS "Detected System Architecture: ${SYSTEM_ARCHITECTURE}")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Google tests require mimimum c++ version 14
# setting minimum c++ version to 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make sure CMake exports its compile commands so things like IntelliSense can detect them in editor
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)

# Detect OS for platform specific compilation
if(UNIX AND NOT APPLE)
	add_compile_definitions(TARGET_OS=Linux _LINUX_TARGET)
elseif(WIN32)
	add_compile_definitions(TARGET_OS=Windows _WIN_TARGET)
elseif(UNIX AND APPLE)
	add_compile_definitions(TARGET_OS=MacOS _MAC_TARGET)
else()
	add_compile_definitions(TARGET_OS=Unkown _UNKNOWN_TARGET)
endif()

# A significant release that likely includes new features, changes, and enhancements that may not be backward compatible with the previous version
add_compile_definitions(PACKAGE_VERSION_MAJOR=1)
# A release that may add new features but remains backward compatible with the previous version
add_compile_definitions(PACKAGE_VERSION_MINOR=0)
# A release that includes minor changes, bug fixes, and patches without adding new features. It is typically backward and forward compatible with the corresponding minor version
add_compile_definitions(PACKAGE_VERSION_PATCH=0)
# A version that is often used to denote a specific build of the software, including revisions, builds, or other metadata
add_compile_definitions(PACKAGE_VERSION_BUILD=0)

set(PACKAGE_VERSION "${PACKAGE_VERSION_MAJOR}.${PACKAGE_VERSION_MINOR}.${PACKAGE_VERSION_PATCH}.${PACKAGE_VERSION_BUILD}")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/GenToolsPackageConfigVersion.cmake"
	VERSION ${PACKAGE_VERSION}
	COMPATIBILITY AnyNewerVersion
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/GenToolsPackageConfigVersion.cmake" DESTINATION "lib/cmake/GenToolsPackage")

# Create a CMake option variable used to trigger settings for the special Release-Dev build mode of this project
option(RELEASE_ERROR_MSG "Include release error messages in the build" OFF)

# Define custom configuration types for Debug, Release, and Release-Dev
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "List of supported build modes." FORCE)

# Create CMake referecne to a directory for configuration templates
set(CONFIG_TEMPLATES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/config_templates)

# Add preprocessor definitions for conditional compilation based on configuration
add_compile_definitions($<$<CONFIG:Debug>:_DEBUG> $<$<CONFIG:RelWithDebInfo>:_RELEASE_DEV> $<$<CONFIG:Release>:_RELEASE>)

# Create CMake reference to a directory for CMake generated files
set(GENERATED_DIR ${CMAKE_SOURCE_DIR}/generated)
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

# Create a set for build all unit tests
set_property(GLOBAL PROPERTY UNIT_TEST_TARGETS "")
set_property(GLOBAL PROPERTY UNIT_TEST_SOURCES "")

# include sub-directories
add_subdirectory(GenParse)

get_property(test_targets GLOBAL PROPERTY UNIT_TEST_TARGETS)
if(test_targets)
	# Create a custom target for all tests
	add_custom_target(all_unit_tests DEPENDS ${test_targets})
endif()

get_property(test_source GLOBAL PROPERTY UNIT_TEST_SOURCES)
if(test_source)
	# Create an executable for the custom target, such that the IDEs can see it as a runnable target
	add_executable(run_all_unit_tests EXCLUDE_FROM_ALL ${test_source})
	# Link the executable with GTest and the TokenValueParser library
	target_link_libraries(run_all_unit_tests PRIVATE GTest::gtest_main GenParseBase GenParseCmdLine)
endif()