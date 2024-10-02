# CMakeLists.txt ConsoleUIToolkit

# Create CMake reference to this source directory, used in sub-directory CMake files
set(ConsoleUIToolkit_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Create list of all source files to be used in this target
file(GLOB_RECURSE ConsoleUIToolkit_SOURCE "${ConsoleUIToolkit_SOURCE_DIR}/src/*.cpp" "${GENERATED_SRC_DIR}/*.cpp")

if(NOT ConsoleUIToolkit_SOURCE)
	set(LIBRARY_TYPE "INTERFACE")
	set(INCLUDE_TYPE "INTERFACE")
	set(COMP_DEF_TYPE "INTERFACE")
else()
	set(LIBRARY_TYPE "STATIC")
	set(INCLUDE_TYPE "PUBLIC")
	set(COMP_DEF_TYPE "PRIVATE")
endif()

# Add GenParseBase library target
add_library(ConsoleUIToolkit ${LIBRARY_TYPE} ${ConsoleUIToolkit_SOURCE})

# Include the project header files in the include directory
target_include_directories(ConsoleUIToolkit ${INCLUDE_TYPE} ${ConsoleUIToolkit_SOURCE_DIR}/include)
# Include the generated headers in the generated directory
target_include_directories(ConsoleUIToolkit ${INCLUDE_TYPE} ${GENERATED_INCLUDE_DIR})

# Install the static library/include the library binaries in the library package
install(TARGETS ConsoleUIToolkit 
	ARCHIVE DESTINATION ConsoleUIToolkit/lib
	LIBRARY DESTINATION ConsoleUIToolkit/lib
	RUNTIME DESTINATION ConsoleUIToolkit/bin
)

# Install the public header files/include public headers in library package
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ 
DESTINATION ConsoleUIToolkit/include 
FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

# Configure CMake for testing. Needed for GTest features
enable_testing()

# Connect the tests subdirectory and its targets to the root target
#add_subdirectory(tests)

# Add versioning info
set(ConsoleUIToolkit_VERSION_MAJOR 1)
set(ConsoleUIToolkit_VERSION_MINOR 0)
set(ConsoleUIToolkit_VERSION_PATCH 0)
set(ConsoleUIToolkit_VERSION_BUILD 0)

target_compile_definitions(ConsoleUIToolkit ${COMP_DEF_TYPE} CONSOLE_UI_TOOLKIT_VERSION_MAJOR=${ConsoleUIToolkit_VERSION_MAJOR} CONSOLE_UI_TOOLKIT_VERSION_MINOR=${ConsoleUIToolkit_VERSION_MINOR} CONSOLE_UI_TOOLKIT_VERSION_PATCH=${ConsoleUIToolkit_VERSION_PATCH} CONSOLE_UI_TOOLKIT_VERSION_BUILD=${ConsoleUIToolkit_VERSION_BUILD})