# CMakeLists.txt GenParseBase

# Create CMake reference to this source directory, used in sub-directory CMake files
set(GenParseBase_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Create list of all source files to be used in this target
file(GLOB_RECURSE GenParseBase_SOURCE "${GenParseBase_SOURCE_DIR}/src/*.cpp" "${GENERATED_SRC_DIR}/*.cpp")

# Add GenParseBase library target
add_library(GenParseBase STATIC ${GenParseBase_SOURCE})

# Include the project header files in the include directory
target_include_directories(GenParseBase PUBLIC ${GenParseBase_SOURCE_DIR}/include)
# Include the generated headers in the generated directory
target_include_directories(GenParseBase PUBLIC ${GENERATED_INCLUDE_DIR})

# Install the static library/include the library binaries in the library package
install(TARGETS GenParseBase 
	ARCHIVE DESTINATION GenParseBase/lib
	LIBRARY DESTINATION GenParseBase/lib
	RUNTIME DESTINATION GenParseBase/bin
)

# Install the public header files/include public headers in library package
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ 
DESTINATION GenParseBase/include 
FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

# Connect the tests subdirectory and its targets to the root target
add_subdirectory(tests EXCLUDE_FROM_ALL)

# Add versioning info
set(GenParseBase_VERSION_MAJOR 1)
set(GenParseBase_VERSION_MINOR 0)
set(GenParseBase_VERSION_PATCH 0)
set(GenParseBase_VERSION_BUILD 0)

target_compile_definitions(GenParseBase PRIVATE GEN_PARSE_BASE_VERSION_MAJOR=${GenParseBase_VERSION_MAJOR} GEN_PARSE_BASE_VERSION_MINOR=${GenParseBase_VERSION_MINOR} GEN_PARSE_BASE_VERSION_PATCH=${GenParseBase_VERSION_PATCH} GEN_PARSE_BASE_VERSION_BUILD=${GenParseBase_VERSION_BUILD})