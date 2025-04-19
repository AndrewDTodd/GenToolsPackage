# CMakeList.txt : GenToolsPackage::GenSerialize::StandardPlugins::JSON

project(GEN_SERIALIZE_JSON VERSION 1.0.0)
set(TARGET_NAME JSON_FormatPlugin)

# Create options that are dependent onthis project being top level
option(${PROJECT_NAME}_VERBOSE "Enable verbose messages for ${TARGET_NAME}" ${PROJECT_IS_TOP_LEVEL})

message(STATUS "${PROJECT_NAME}_VERBOSE: ${${PROJECT_NAME}_VERBOSE}")

# Target Creation *********************************************************************************
#**************************************************************************************************

option(${PROJECT_NAME}_DEBUG "Enable CMake related Debug messages" OFF)

file(GLOB_RECURSE ${TARGET_NAME}_SOURCE 
	"${CMAKE_SOURCE_DIR}/generated/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/*/src/*.cpp"
)

file(GLOB ${TARGET_NAME}_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
list(APPEND ${TARGET_NAME}_DIRS ".")

if(${PROJECT_NAME}_DEBUG)
	message(STATUS "${TARGET_NAME}_DIRS: ${${TARGET_NAME}_DIRS}")
	message(STATUS "${TARGET_NAME}_SOURCE: ${${TARGET_NAME}_SOURCE}")
endif()

if(NOT DEFINED ${PROJECT_NAME}_BUILD)
	set(${PROJECT_NAME}_BUILD ON)
endif()

if(${PROJECT_NAME}_BUILD)
	# Create the RenderingPrimities target
	if(${TARGET_NAME}_SOURCE)
		if(${PROJECT_NAME}_DEBUG)
			message(STATUS "Creating target: ${TARGET_NAME} as SHARED library")
		endif()

		add_library(${TARGET_NAME} SHARED ${${TARGET_NAME}_SOURCE})

		# Link libraries to the target
		target_link_libraries(${TARGET_NAME} LibGenSerialize)
		
		# Set the FORMAT_PLUGIN_EXPORTS macro for JSON_Format_Plugin
		target_compile_definitions(${TARGET_NAME} PRIVATE FORMAT_PLUGIN_EXPORTS)

		if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/generated/include")
			if(${PROJECT_NAME}_DEBUG)
				message(STATUS "Adding include directory: ${CMAKE_SOURCE_DIR}/generated/include")
			endif()
			target_include_directories(${TARGET_NAME} PUBLIC 
				$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/generated/include> 
 				$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/${dir}> # This is used when the library is installed
			)
		endif()

		# Function to recursively get all subdirectories
		function(get_all_subdirectories BASE_DIR OUT_VAR)
			file(GLOB_RECURSE SUBDIRS LIST_DIRECTORIES true "${BASE_DIR}/*")

			set(DIR_LIST "")
			foreach(SUBDIR ${SUBDIRS})
				if(IS_DIRECTORY ${SUBDIR})
					list(APPEND DIR_LIST ${SUBDIR})
				endif()
			endforeach()

			set(${OUT_VAR} ${DIR_LIST} PARENT_SCOPE)
		endfunction()

		# Set up include directories for the library target
		foreach(dir ${${TARGET_NAME}_DIRS})
			set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include")
			if(IS_DIRECTORY "${INCLUDE_DIR}")
				if(${PROJECT_NAME}_DEBUG)
					message(STATUS "Adding include directory: ${INCLUDE_DIR}")
				endif()

				# Get all subdirectories
				get_all_subdirectories(${INCLUDE_DIR} ALL_INCLUDE_DIRS)

				# Add include directories to the target
				target_include_directories(${TARGET_NAME} PUBLIC 
					$<BUILD_INTERFACE:${INCLUDE_DIR}>
					$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/${dir}>
				)
        
				# Add all subdirectories
				foreach(subdir ${ALL_INCLUDE_DIRS})
					target_include_directories(${TARGET_NAME} PUBLIC 
						$<BUILD_INTERFACE:${subdir}>
						$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/${dir}>
					)
				endforeach()
			endif()

			set(INL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inl")
			if(IS_DIRECTORY "${INL_DIR}")
				if(${PROJECT_NAME}_DEBUG)
					message(STATUS "Adding inl directory: ${INL_DIR}")
				endif()
				
				# Get all subdirectories
				get_all_subdirectories(${INL_DIR} ALL_INL_DIRS)

				# Add include directories to the target
				target_include_directories(${TARGET_NAME} PUBLIC 
					$<BUILD_INTERFACE:${INL_DIR}>
					$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/${dir}>
				)
        
				# Add all subdirectories
				foreach(subdir ${ALL_INL_DIRS})
					target_include_directories(${TARGET_NAME} PUBLIC 
						$<BUILD_INTERFACE:${subdir}>
						$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/${dir}>
					)
				endforeach()
			endif()
		endforeach()
	endif()

	set_target_properties(${TARGET_NAME} PROPERTIES 
		VERSION ${PROJECT_VERSION} 
		SOVERSION ${PROJECT_VERSION_MAJOR}
	)

# End Target Creation *****************************************************************************
#**************************************************************************************************

# Installation and Packing Configuration **********************************************************
#**************************************************************************************************

	# Install the targets
	install(
		TARGETS ${TARGET_NAME} 
		EXPORT ${TARGET_NAME}_Targets 
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} # Static libraries/import libraries (.lib files for .dll linking) 
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} # Shared libraries (.so) 
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # .exe or .dll 
		PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # Headers/include directories marked as PUBLIC 
		PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} # Headers/include directories marked as PRIVATE
	)

	# Create the targets CMake file which contains the above definitions
	install(
		EXPORT ${TARGET_NAME}_Targets 
		FILE ${TARGET_NAME}_Targets.cmake 
		NAMESPACE GenToolsPackage::${TARGET_NAME}::
		DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${TARGET_NAME}
	)

	if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/generated/include")
		install(
			DIRECTORY "${CMAKE_SOURCE_DIR}/generated/include"
			DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/generated"
		)
	endif()

	# Install the actual includes
	foreach(dir ${${TARGET_NAME}_DIRS})
		if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include")
			install(
				DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include/"
				DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/${dir}"
			)
		endif()

		if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inl")
			install(
				DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inl/"
				DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/${dir}"
			)
		endif()
	endforeach()

	# Generate and install the package version config files
	include(CMakePackageConfigHelpers)
	write_basic_package_version_file(
		"${TARGET_NAME}_ConfigVersion.cmake" 
		VERSION ${PROJECT_VERSION} 
		COMPATIBILITY SameMajorVersion
	)
	configure_package_config_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake_config/${TARGET_NAME}_Config.cmake.in" 
		"${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_Config.cmake" 
		INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${TARGET_NAME}
	)

	# Install the CMake config files
	install(
		FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_ConfigVersion.cmake" 
		"${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}_Config.cmake" 
		DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${TARGET_NAME}
	)

	# Define Package install paths
	set(INCLUDEDIR_FOR_PKG_CONFIG "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}")
	set(LIBDIR_PKG_CONFIG "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

	# Create and install the package config file
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/cmake_config/${TARGET_NAME}.pc.in" 
		"${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.pc" @ONLY
	)

	# Install the package config file
	install(
		FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.pc" 
		DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
	)
endif()

# A version that is often used to denote a specific build of the software, including revisions, builds, or other metadata
set(PACKAGE_VERSION_BUILD "${CMAKE_SYSTEM_PROCESSOR}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")

set(PACKAGE_VERSION "${PROJECT_VERSION}-${PACKAGE_VERSION_BUILD}")

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_SOURCE_DIR}/out/package")

set(CPACK_PACKAGE_NAME "${TARGET_NAME}")
set(CPACK_PACKAGE_VERSION "${PACKAGE_VERSION}")

set(CPACK_PACKAGE_VENDOR "Andrew Todd")
set(CPACK_PACKAGE_CONTACT "andrewdanieltodd@gmail.com")
include(CPack)

if(RENDERING_PRIMITIVES_VERBOSE)
	message(STATUS "PACKAGE_VERSION is: ${PACKAGE_VERSION}")
	message(STATUS "PACKAGE_FILE_NAME is: ${CPACK_PACKAGE_FILE_NAME}")
endif()

# End Installation and Packing Configuration ******************************************************
#**************************************************************************************************

# Create Unit Test Groups *************************************************************************
#**************************************************************************************************
if (GEN_TOOLS_PACKAGE_BUILD_TESTS)
	if(${PROJECT_NAME}_DEBUG)
		message(STATUS "Building test suit for ${TARGET_NAME}")
	endif()

	set(${TARGET_NAME}_TEST_DIRS "")

	foreach(dir ${${TARGET_NAME}_DIRS})
		if(IS_DIRECTORY "${dir}/tests")
			list(APPEND ${TARGET_NAME}_TEST_DIRS "${dir}/tests")
		endif()
		if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/tests")
				if(${PROJECT_NAME}_DEBUG)	
					message(STATUS "Adding test directory: ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/tests")
				endif()
				list(APPEND ${TARGET_NAME}_TEST_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/tests")
			endif()
	endforeach()

	# Do not install GTest when packaging targets
	set(INSTALL_GTEST OFF)
	
	# Add all the tests directories
	foreach(tests_dir ${${TARGET_NAME}_TEST_DIRS})
		if(${PROJECT_NAME}_DEBUG)
			message(STATUS "Adding Sub-Directory: ${tests_dir}")
		endif()
		add_subdirectory("${tests_dir}")
	endforeach()
endif()
# End Create Unit Test Groups *********************************************************************
#**************************************************************************************************

# Determine the location of the build shared library
add_custom_command(TARGET JSON_FormatPlugin POST_BUILD 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different 
	$<TARGET_FILE:JSON_FormatPlugin> # The build shared library
	$<TARGET_FILE_DIR:run_JSONFormatPlugin_tests> # The directory where the test executalbles are 
)