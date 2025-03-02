# GenParseBase tests target section
################################################################################################################################################################
# Installation and setup of the gTest suite
# Build a tests executable for the execution of the projects tests
include(FetchContent)
FetchContent_Declare(
	googletest
	DOWNLOAD_EXTRACT_TIMESTAMP true
	URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

set(INSTALL_GTEST OFF)

# New format for including googletest subdirectory. To prevent googletest items being added to install
FetchContent_MakeAvailable(googletest)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()
include(GoogleTest)

get_property(existing_sources GLOBAL PROPERTY UNIT_TEST_SOURCES)

# Get a list of all the test related .cpp files in the unit tests subdirectory
file(GLOB_RECURSE GenParseBase_UnitTest_Sources "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp")

list(APPEND existing_sources ${GenParseBase_UnitTest_Sources})

set_property(GLOBAL PROPERTY UNIT_TEST_SOURCES "${existing_sources}")

get_property(UNIT_TEST_TARGETS GLOBAL PROPERTY UNIT_TEST_TARGETS)

# Create a set for the Flag aggregate types unit tests
set(FLAG_AGGREGATE_TYPES_TARGETS)
# Get a list of the .cpp files in the subdirectory for the unit tests
file(GLOB_RECURSE FLAG_AGGREGATE_TYPES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/flag_aggregate/*.cpp")

# Add each source file as a test target
foreach(TEST_SOURCE ${FLAG_AGGREGATE_TYPES_SOURCES})
	get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
	add_executable(${TEST_NAME} EXCLUDE_FROM_ALL ${TEST_SOURCE})
	target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main GenParse)
	set_target_properties(${TEST_NAME} PROPERTIES INSTALLABLE OFF)
	list(APPEND FLAG_AGGREGATE_TYPES_TARGETS ${TEST_NAME})
	list(APPEND UNIT_TEST_TARGETS ${TEST_NAME})
	gtest_discover_tests(${TEST_NAME} PROPERTIES LABELS "FlagAggregates")
endforeach()

# Create a set for the Flag aggregate types unit tests
set(PARSE_FUNCTIONS_TARGETS)
# Get a list of the .cpp files in the subdirectory for the unit tests
file(GLOB PARSE_FUNCTIONS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/parse_functions/*.cpp")

# Add each source file as a test target
foreach(TEST_SOURCE ${PARSE_FUNCTIONS_SOURCES})
	get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
	add_executable(${TEST_NAME} EXCLUDE_FROM_ALL ${TEST_SOURCE})
	target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main GenParse)
	set_target_properties(${TEST_NAME} PROPERTIES INSTALLABLE OFF)
	list(APPEND PARSE_FUNCTIONS_TARGETS ${TEST_NAME})
	list(APPEND UNIT_TEST_TARGETS ${TEST_NAME})
	gtest_discover_tests(${TEST_NAME} PROPERTIES LABELS "ParseFunctions")
endforeach()

# Create a set for the Flag aggregate types unit tests
set(FLAG_TYPES_TARGETS)
# Get a list of the .cpp files in the subdirectory for the unit tests
file(GLOB_RECURSE FLAG_TYPES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/flag/*.cpp")

# Add each source file as a test target
foreach(TEST_SOURCE ${FLAG_TYPES_SOURCES})
	get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
	add_executable(${TEST_NAME} EXCLUDE_FROM_ALL ${TEST_SOURCE})
	target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main GenParse)
	set_target_properties(${TEST_NAME} PROPERTIES INSTALLABLE OFF)
	list(APPEND FLAG_TYPES_TARGETS ${TEST_NAME})
	list(APPEND UNIT_TEST_TARGETS ${TEST_NAME})
	gtest_discover_tests(${TEST_NAME} PROPERTIES LABELS "FlagTypes")
endforeach()

# Create a custom target for the flag_aggregate_types tests
add_custom_target(flag_aggregate_tests DEPENDS ${FLAG_AGGREGATE_TYPES_TARGETS})
# Create an executable for the custom target, such that the IDEs can see it as a runnable target
add_executable(run_flag_aggregate_tests EXCLUDE_FROM_ALL ${FLAG_AGGREGATE_TYPES_SOURCES})
# Link the executable with GTest and the TokenValueParser library
target_link_libraries(run_flag_aggregate_tests PRIVATE GTest::gtest_main GenParse)
set_target_properties(run_flag_aggregate_tests PROPERTIES INSTALLABLE OFF)

# Create a custom target for the parse_functions tests
add_custom_target(parse_functions_tests DEPENDS ${PARSE_FUNCTIONS_TARGETS})
# Create an executable for the custom target, such that the IDEs can see it as a runnable target
add_executable(run_parse_functions_tests EXCLUDE_FROM_ALL ${PARSE_FUNCTIONS_SOURCES})
# Link the executable with GTest and the TokenValueParser library
target_link_libraries(run_parse_functions_tests PRIVATE GTest::gtest_main GenParse)
set_target_properties(run_parse_functions_tests PROPERTIES INSTALLABLE OFF)

# Create a custom target for the flags types
add_custom_target(flags_tests DEPENDS ${FLAG_TYPES_TARGETS})
# Create an executable for the custom target, such that the IDEs can see it as a runnable target
add_executable(run_flags_tests EXCLUDE_FROM_ALL ${FLAG_TYPES_SOURCES})
# Link the executable with GTest and the TokenValueParser library
target_link_libraries(run_flags_tests PRIVATE GTest::gtest_main GenParse)
set_target_properties(run_flags_tests PROPERTIES INSTALLABLE OFF)

set_property(GLOBAL PROPERTY UNIT_TEST_TARGETS "${UNIT_TEST_TARGETS}")
################################################################################################################################################################

#add tests to be discoverable by ctest *Note this is only necessary when not using gtest_discover. The tests are automatically added by gtest
################################################################################################################################################################
#add_test(NAME FlagArgument_UnitTests COMMAND FlagArgument_UnitTests)