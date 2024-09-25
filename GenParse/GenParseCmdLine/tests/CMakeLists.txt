# GenParseCmdLine tests target section
################################################################################################################################################################
# Installation and setup of the gTest suite
# Build a tests executable for the execution of the projects tests
include(FetchContent)
FetchContent_Declare(
	googletest
	DOWNLOAD_EXTRACT_TIMESTAMP true
	URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

# New format for including googletest subdirectory. To prevent googletest items being added to install
FetchContent_MakeAvailable(googletest)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()
include(GoogleTest)

get_property(existing_sources GLOBAL PROPERTY UNIT_TEST_SOURCES)

# Get a list of all the test related .cpp files in the unit tests subdirectory
file(GLOB_RECURSE GenParseCmdLine_UnitTest_Sources "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp")

list(APPEND existing_sources ${GenParseCmdLine_UnitTest_Sources})

set_property(GLOBAL PROPERTY UNIT_TEST_SOURCES "${existing_sources}")

get_property(UNIT_TEST_TARGETS GLOBAL PROPERTY UNIT_TEST_TARGETS)

# Create a set for the Flag aggregate types unit tests
set(FLAG_DERIVED_TYPES_TARGETS)
# Get a list of the .cpp files in the subdirectory for the unit tests
file(GLOB_RECURSE FLAG_DERIVED_TYPES_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/unit/FlagDerivedTypes/*.cpp")

# Add each source file as a test target
foreach(TEST_SOURCE ${FLAG_DERIVED_TYPES_SOURCES})
	get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
	add_executable(${TEST_NAME} EXCLUDE_FROM_ALL ${TEST_SOURCE})
	target_link_libraries(${TEST_NAME} PRIVATE GTest::gtest_main GenParseCmdLine)
	#target_include_directories(${TEST_NAME} PRIVATE ${googletest_SOURCE_DIR}/googletest/include)
	#target_include_directories(${TEST_NAME} PRIVATE ${GenParseCmdLine_SOURCE_DIR}/include)
	list(APPEND FLAG_TYPES_TARGETS ${TEST_NAME})
	list(APPEND UNIT_TEST_TARGETS ${TEST_NAME})
	gtest_discover_tests(${TEST_NAME} PROPERTIES LABELS "FlagDerivedTypes")
	#set_target_properties(${TEST_NAME} PROPERTIES FOLDER Tests/flags)
endforeach()

# Create a custom target for the flag_aggregate_types tests
add_custom_target(flag_derived_tests DEPENDS ${FLAG_DERIVED_TYPES_TARGETS})
# Create an executable for the custom target, such that the IDEs can see it as a runnable target
add_executable(run_flag_derived_tests EXCLUDE_FROM_ALL ${FLAG_DERIVED_TYPES_SOURCES})
# Link the executable with GTest and the TokenValueParser library
target_link_libraries(run_flag_derived_tests PRIVATE GTest::gtest_main GenParseCmdLine)

set_property(GLOBAL PROPERTY UNIT_TEST_TARGETS "${UNIT_TEST_TARGETS}")
################################################################################################################################################################

#add tests to be discoverable by ctest *Note this is only necessary when not using gtest_discover. The tests are automatically added by gtest
################################################################################################################################################################
#add_test(NAME FlagArgument_UnitTests COMMAND FlagArgument_UnitTests)