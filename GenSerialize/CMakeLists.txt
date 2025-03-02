# CMakeList.txt : GenToolsPackage::GenSerialize CMake project

project(GEN_SERIALIZE VERSION 1.0.0)
set(TARGET_NAME GenSerialize)

# Create options that are dependent onthis project being top level
option(${PROJECT_NAME}_VERBOSE "Enable verbose messages for ${TARGET_NAME}" ${PROJECT_IS_TOP_LEVEL})

message(STATUS "${PROJECT_NAME}_VERBOSE: ${${PROJECT_NAME}_VERBOSE}")

option(${PROJECT_NAME}_DEBUG "Enable CMake related Debug messages" OFF)

# FIND PACKAGE and FETCH CONTENT ******************************************************************
#**************************************************************************************************

# Check for a system-level Clang installation
find_package(Clang QUIET)

if (Clang_FOUND)
    if (${PROJECT_NAME}_DEBUG)
        message(STATUS "System Clang installation found; will use system Clang.")
    endif()
    set(USE_SYSTEM_CLANG TRUE)
else()
    if (${PROJECT_NAME}_DEBUG)
        message(STATUS "System Clang not found; will use clone and build Clang.")
    endif()
    set(USE_SYSTEM_CLANG FALSE)
endif()

if (NOT USE_SYSTEM_CLANG)
    # Set the directory where the minimal LLVM/Clang source will be placed.
    set(LLVM_CLANG_MINIMAL_DIR "${CMAKE_SOURCE_DIR}/llvm-clang-minimal")

    # Only perform the clone if the directory does not already exist.
    if(NOT EXISTS "${LLVM_CLANG_MINIMAL_DIR}/clang")
        message(STATUS "Cloning a minimal LLVM/Clang repository for Clang-only build...")

        # Clone the repository shallowly with no checkout and with filtering to reduce data.
        execute_process(
            COMMAND git clone --depth 1 --no-checkout --filter=tree:0 --progress --branch llvmorg-19.1.7 https://github.com/llvm/llvm-project.git ${LLVM_CLANG_MINIMAL_DIR}
            RESULT_VARIABLE GIT_CLONE_RESULT
            ERROR_VARIABLE GIT_CLONE_ERROR
        )
        if(NOT GIT_CLONE_RESULT EQUAL 0)
            message(FATAL_ERROR "Failed to clone LLVM repository: ${GIT_CLONE_ERROR}")
        endif()

        # Optionally, configure Git to skip fetching user and revert branches.
        execute_process(
            COMMAND git -C ${LLVM_CLANG_MINIMAL_DIR} config --add remote.origin.fetch ^refs/heads/users/*
            RESULT_VARIABLE _refspec_result_users
        )
        execute_process(
            COMMAND git -C ${LLVM_CLANG_MINIMAL_DIR} config --add remote.origin.fetch ^refs/heads/revert-*
            RESULT_VARIABLE _refspec_result_revert
        )

        # Check out the code.
        execute_process(
            COMMAND git -C ${LLVM_CLANG_MINIMAL_DIR} checkout llvmorg-19.1.7
            RESULT_VARIABLE GIT_CHECKOUT_RESULT
            ERROR_VARIABLE GIT_CHECKOUT_ERROR
            OUTPUT_VARIABLE GIT_CHECKOUT_OUTPUT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )

        # Print the output from the checkout process
        message(STATUS "Git Checkout Output: ${GIT_CHECKOUT_OUTPUT}")
        message(STATUS "Git Checkout Errors: ${GIT_CHECKOUT_ERROR}")

        if (GIT_CHECKOUT_RESULT)
            message(FATAL_ERROR "Git checkout failed with error: ${GIT_CHECKOUT_ERROR}")
        endif()
    endif()

    # Determine architecture for LLVM build
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(LLVM_TARGET_ARCH "X86")
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|ARM64")
        set(LLVM_TARGET_ARCH "AArch64")
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
        set(LLVM_TARGET_ARCH "ARM")
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "ppc64|powerpc64")
        set(LLVM_TARGET_ARCH "PowerPC")
    elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64")
        set(LLVM_TARGET_ARCH "RISCV")
    else()
        set(LLVM_TARGET_ARCH "X86") # Default fallback
        message(WARNING "Unknown architecture '${CMAKE_SYSTEM_PROCESSOR}', defaulting to X86 for LLVM.")
    endif()

    # Define directories and a marker file
    set(LLVM_SOURCE_DIR "${LLVM_CLANG_MINIMAL_DIR}/llvm")
    set(LLVM_BUILD_DIR "${CMAKE_BINARY_DIR}/llvm-build/$<CONFIG>")
    set(LLVM_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/llvm-install/$<CONFIG>")
    set(LLVM_BUILT_FLAG "${LLVM_INSTALL_DIR}/built.flag")

    #[[# Single custom command that configures, builds, and installs LLVM/Clang
    add_custom_command(
      OUTPUT ${LLVM_BUILT_FLAG}

      # Ensure the build directory exists; the generator expression $<CONFIG> is evaluated here.
      COMMAND ${CMAKE_COMMAND} -E make_directory ${LLVM_BUILD_DIR}

      # Configure step (run from LLVM_BUILD_DIR)
      COMMAND ${CMAKE_COMMAND} -E chdir ${LLVM_BUILD_DIR} ${CMAKE_COMMAND} -E echo "Configuring LLVM/Clang..."
      COMMAND ${CMAKE_COMMAND} -E chdir ${LLVM_BUILD_DIR} ${CMAKE_COMMAND} -Wno-dev -GNinja
        -DCMAKE_INSTALL_PREFIX=${LLVM_INSTALL_DIR}
        -DLLVM_ENABLE_PROJECTS=clang
        -DLLVM_TARGETS_TO_BUILD=${LLVM_TARGET_ARCH}
        -DLLVM_PARALLEL_LINK_JOBS=6
        -DLLVM_ENABLE_ASSERTIONS=OFF
        -DLLVM_BUILD_LLVM_DYLIB=OFF
        -DLLVM_USE_LINKER=lld
        -DCMAKE_BUILD_TYPE=$<CONFIG>
        ${LLVM_SOURCE_DIR}
      COMMENT "Configuring LLVM/Clang (if needed)..."
      VERBATIM

      # Build step (run from LLVM_BUILD_DIR)
      COMMAND ${CMAKE_COMMAND} -E chdir ${LLVM_BUILD_DIR} ${CMAKE_COMMAND} -E echo "Building LLVM/Clang..."
      COMMAND ${CMAKE_COMMAND} -E chdir ${LLVM_BUILD_DIR} ninja

      # Install step (run from LLVM_BUILD_DIR)
      COMMAND ${CMAKE_COMMAND} -E chdir ${LLVM_BUILD_DIR} ${CMAKE_COMMAND} -E echo "Installing LLVM/Clang..."
      COMMAND ${CMAKE_COMMAND} -E chdir ${LLVM_BUILD_DIR} ninja install

      # Touch file to indicate successful install
      COMMAND ${CMAKE_COMMAND} -E chdir ${LLVM_BUILD_DIR} ${CMAKE_COMMAND} -E touch ${LLVM_BUILT_FLAG}
      COMMENT "Building and Installing LLVM/Clang (if needed)..."
    )

    add_custom_target(llvm-install DEPENDS ${LLVM_BUILT_FLAG})]]

    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    # Custom command to build clang only if it hasn’t been built already.
    add_custom_command(
      OUTPUT ${LLVM_BUILT_FLAG}
      COMMAND ${CMAKE_COMMAND} -E echo "Checking if Clang is built for configuration $<CONFIG>..."
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake_config/scripts/build_clang.py
              ${LLVM_BUILT_FLAG} ${LLVM_BUILD_DIR} ${LLVM_INSTALL_DIR} ${LLVM_SOURCE_DIR} $<CONFIG> ${LLVM_TARGET_ARCH}
      COMMENT "Building and Installing LLVM/Clang (if needed)..."
      VERBATIM
    )

    # Custom target that depends on the stamp file.
    add_custom_target(llvm-install ALL DEPENDS ${LLVM_BUILT_FLAG})

    # Set include and lib paths for built Clang
    set(LLVM_INCLUDE_DIR "${LLVM_INSTALL_DIR}/include")
    set(LLVM_LIB_DIR "${LLVM_INSTALL_DIR}/lib")
endif()

# End FIND PACKAGE and FETCH CONTENT **************************************************************
#**************************************************************************************************

# Target Creation *********************************************************************************
#**************************************************************************************************

file(GLOB_RECURSE ${TARGET_NAME}_SOURCE 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/generated/src/*.cpp"
)

file(GLOB ${TARGET_NAME}_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
list(APPEND ${TARGET_NAME}_DIRS ".")
list(APPEND ${TARGET_NAME}_DIRS "${CMAKE_SOURCE_DIR}/generated")

if (${PROJECT_NAME}_DEBUG)
    message(STATUS "${TARGET_NAME}_DIRS: ${${TARGET_NAME}_DIRS}")
    message(STATUS "${TARGET_NAME}_SOURCE: ${${TARGET_NAME}_SOURCE}")
endif()

if (NOT DEFINED ${PROJECT_NAME}_BUILD)
    set(${PROJECT_NAME}_BUILD ON)
endif()

if (${PROJECT_NAME}_BUILD)
    if (${TARGET_NAME}_SOURCE)
        if (${PROJECT_NAME}_DEBUG)
            message(STATUS "Creating target: ${TARGET_NAME} executable")
        endif()

        add_executable(${TARGET_NAME} ${${TARGET_NAME}_SOURCE})

        target_link_libraries(${TARGET_NAME} PRIVATE GenParse)

        # Only add llvm-clang dependency if using an external build
        if (NOT USE_SYSTEM_CLANG)
            # Ensure Clang fetch completes before compiling
            add_dependencies(${TARGET_NAME} llvm-install)

            # Add Clang headers and libraries for linking
            target_include_directories(${TARGET_NAME} PRIVATE ${LLVM_INCLUDE_DIR})
            target_link_directories(${TARGET_NAME} PRIVATE ${LLVM_LIB_DIR})

            target_link_libraries(${TARGET_NAME} PRIVATE 
            clangFrontend 
            clangAST 
            clangBasic 
            clangLex 
            
            LLVMCore 
            LLVMFrontendOpenMP 
            LLVMFrontendOffloading
            LLVMScalarOpts
            LLVMInstCombine
            LLVMAggressiveInstCombine
            LLVMTransformUtils
            LLVMAnalysis
            LLVMProfileData
            LLVMSymbolize
            LLVMDebugInfoBTF
            LLVMDebugInfoPDB
            LLVMDebugInfoMSF
            LLVMDebugInfoDWARF
            LLVMObject
            LLVMTextAPI
            LLVMMCParser
            LLVMIRReader
            LLVMAsmParser
            LLVMMC
            LLVMDebugInfoCodeView
            LLVMBitReader
            LLVMRemarks 
            LLVMBitstreamReader 
            LLVMBinaryFormat 
            LLVMTargetParser 
            LLVMSupport 
            LLVMDemangle

            ntdll
            )
        else()
            # Use system Clang's include and library paths
            target_include_directories(${TARGET_NAME} PRIVATE ${Clang_INCLUDE_DIRS})
            target_link_libraries(${TARGET_NAME} PRIVATE clangFrontend clangAST clangBasic)
        endif()

        # Set up include directories
        foreach (dir ${${TARGET_NAME}_DIRS})
            if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include")
                if (${PROJECT_NAME}_DEBUG)
                    message(STATUS "Adding include directory: ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include")
                endif()
                target_include_directories(${TARGET_NAME} PRIVATE 
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include> 
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/${dir}> # Used when installed
                )
            endif()

            if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inl")
                if (${PROJECT_NAME}_DEBUG)
                    message(STATUS "Adding inl directory: ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inl")
                endif()
                target_include_directories(${TARGET_NAME} PRIVATE 
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inl> 
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}/${dir}> # Used when installed
                )
            endif()
        endforeach()

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
		    NAMESPACE GenToolsPackage::${TARGET_NAME}
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

# Custom Commands *********************************************************************************
#**************************************************************************************************

        # Allow the user or parent project to specify which source files to process via a cache variable.
        set(GEN_SERIALIZE_SOURCE_FILES "" CACHE STRING "List of source files to run the GenSerialize tool on")

        # Get the name of the top-level project (if it is GenSerialize or GenPackage, don't run this tool)
        if (CMAKE_PROJECT_NAME STREQUAL "GenSerialize" OR CMAKE_PROJECT_NAME STREQUAL "GenPackage")
            message(STATUS "Skipping GenSerializeTool scanning: Running inside ${CMAKE_PROJECT_NAME}")
            return()
        endif()

        if (GEN_SERIALIZE_SOURCE_FILES STREQUAL "")
            message(STATUS "Gathering source files from CMake cache...")

            # Collect source files from all targets in the root project
            get_directory_property(ALL_TARGETS DIRECTORY ${CMAKE_SOURCE_DIR} BUILDSYSTEM_TARGETS)
            set(FOUND_SOURCE_FILES "")

            foreach(target IN LISTS ALL_TARGETS)
                get_target_property(TARGET_SOURCES ${target} SOURCES)
        
                if (TARGET_SOURCES)
                    foreach(source IN LISTS TARGET_SOURCES)
                        # Ensure the file is inside the root source directory, not in the build directory,
                        # and is not from GenSerialize or GenPackage
                        if ("${source}" MATCHES "^${CMAKE_SOURCE_DIR}/.*"
                            AND NOT "${source}" MATCHES "^${CMAKE_BINARY_DIR}/.*"
                            AND NOT "${source}" MATCHES ".*/GenSerialize/.*"
                            AND NOT "${source}" MATCHES ".*/GenPackage/.*")
                            list(APPEND FOUND_SOURCE_FILES ${source})
                        endif()
                    endforeach()
                endif()
            endforeach()

            # Remove duplicates
            list(REMOVE_DUPLICATES FOUND_SOURCE_FILES)
            set(GEN_SERIALIZE_SOURCE_FILES "${FOUND_SOURCE_FILES}" CACHE STRING "Tracked source files" FORCE)

	        if(${PROJECT_NAME}_DEBUG)
		        message(STATUS "Source being sent to GenSerialize: ${GEN_SERIALIZE_SOURCE_FILES}")
	        endif()
        endif()

        # Define a directory for generated files.
        set(GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")
        file(MAKE_DIRECTORY ${GENERATED_DIR})

        # Create a custom command that runs GenSerializeTool to process the source files.
        add_custom_command(
            OUTPUT ${CMAKE_BINARY_DIR}/gen_serialize_stamp
            COMMAND ${TARGET_NAME}
                    --input ${GEN_SERIALIZE_SOURCE_FILES}
                    --output ${GENERATED_DIR}
                    --cache ${CMAKE_BINARY_DIR}/GenSerializeCache.txt
            COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_BINARY_DIR}/gen_serialize_stamp
            DEPENDS ${GEN_SERIALIZE_SOURCE_FILES} ${TARGET_NAME}
            COMMENT "Running GenSerialize on source files..."
        )

        # Create a custom target that depends on the stamp file so that generated code is always updated.
        add_custom_target(GenerateSerializationCode ALL
            DEPENDS ${CMAKE_BINARY_DIR}/gen_serialize_stamp
        )

# End Custom Commands *****************************************************************************
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