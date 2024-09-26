# CMakeLists.txt GenParseCmdLine

# Create CMake reference to this source directory, used in sub-directory CMake files
set(GenParseCmdLine_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Create list of all source files to be used in this target
file(GLOB_RECURSE GenParseCmdLine_SOURCE "${GenParseCmdLine_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/generated/src/*.cpp")

add_library(GenParseCmdLine STATIC ${GenParseCmdLine_SOURCE})

# Include the project header files in the include directory
target_include_directories(GenParseCmdLine PUBLIC ${GenParseCmdLine_SOURCE_DIR}/include)
target_include_directories(GenParseCmdLine PUBLIC ${GenParseCmdLine_SOURCE_DIR}/include/FlagDerivedTypes)
# Include the generated headers in the generated directory
target_include_directories(GenParseCmdLine PUBLIC ${GENERATED_INCLUDE_DIR})

# Link GenParseCmdLine to GenParseBase
target_link_libraries(GenParseCmdLine PUBLIC GenParseBase)

# Install the static library/include the library binaries in the library package
install(TARGETS GenParseCmdLine 
	ARCHIVE DESTINATION GenParseCmdLine/lib
	LIBRARY DESTINATION GenParseCmdLine/lib
	RUNTIME DESTINATION GenParseCmdLine/bin
)

# Install the public header files/include public headers in library package
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ 
DESTINATION GenParseCmdLine/include 
FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

# Configure CMake for testing. Needed for GTest features
enable_testing()

# Connect the tests subdirectory and its targets to the root target
add_subdirectory(tests)

# Add versioning info
set(GenParseCmdLine_VERSION_MAJOR 1)
set(GenParseCmdLine_VERSION_MINOR 0)
set(GenParseCmdLine_VERSION_PATCH 0)
set(GenParseCmdLine_VERSION_BUILD 0)

target_compile_definitions(GenParseBase PRIVATE GEN_PARSE_CMD_LINE_VERSION_MAJOR=${GenParseCmdLine_VERSION_MAJOR} GEN_PARSE_CMD_LINE_VERSION_MINOR=${GenParseCmdLine_VERSION_MINOR} GEN_PARSE_CMD_LINE_VERSION_PATCH=${GenParseCmdLine_VERSION_PATCH} GEN_PARSE_CMD_LINE_VERSION_BUILD=${GenParseCmdLine_VERSION_BUILD})