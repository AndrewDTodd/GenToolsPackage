# CMakeList.txt : GenToolsPackage::GenSerialize CMake project

project(GEN_SERIALIZE VERSION 1.0.0)
set(TARGET_NAME GenSerialize)

# Create options that are dependent onthis project being top level
option(${PROJECT_NAME}_VERBOSE "Enable verbose messages for ${TARGET_NAME}" ${PROJECT_IS_TOP_LEVEL})

message(STATUS "${PROJECT_NAME}_VERBOSE: ${${PROJECT_NAME}_VERBOSE}")

option(${PROJECT_NAME}_DEBUG "Enable CMake related Debug messages" OFF)

# For MSVC: ensure Debug builds use the release STL runtime, to match Clang libs
if (MSVC)
  add_compile_definitions($<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>)
  add_compile_options($<$<CONFIG:Debug>:/MD>)  # Use DLL runtime instead of /MDd
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

# FIND PACKAGE and FETCH CONTENT ******************************************************************
#**************************************************************************************************

# Check for a system-level Clang installation
find_package(Clang QUIET)

if (Clang_FOUND)
    message(STATUS "System Clang installation found; will use system Clang.")
    set(USE_SYSTEM_CLANG TRUE)
else()
    message(STATUS "System Clang not found; will clone and build Clang.")
    set(USE_SYSTEM_CLANG FALSE)
endif()

if (NOT USE_SYSTEM_CLANG)
    # Determine install prefix based on OS and cache it for reuse
    if (WIN32)
        set(CLANG_INSTALL_DIR "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:$ENV{ProgramData}/LLVM/RelWithDebInfo>$<$<CONFIG:Release>:$ENV{ProgramData}/LLVM/Release>" CACHE PATH "Clang install base on Windows")
    else()
        set(CLANG_INSTALL_DIR "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:/usr/local/LLVM/RelWithDebInfo>$<$<CONFIG:Release>:/usr/local/LLVM/Release>" CACHE PATH "Clang install base on Unix")
    endif()

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

    find_package(Python3 COMPONENTS Interpreter REQUIRED)

    add_custom_target(llvm-install ALL
      COMMAND ${CMAKE_COMMAND} -E echo "Checking if Clang is built for configuration $<CONFIG>..."
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/cmake_config/scripts/build_clang.py
              ${CLANG_INSTALL_DIR} ${LLVM_SOURCE_DIR} $<CONFIG> ${LLVM_TARGET_ARCH}
      COMMENT "Building and Installing LLVM/Clang (if needed)..."
      VERBATIM
    )

    # Set include and lib paths for built Clang
    if (WIN32)
        set(LLVM_INCLUDE_DIR "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:$ENV{ProgramData}/LLVM/RelWithDebInfo/include>$<$<CONFIG:Release>:$ENV{ProgramData}/LLVM/Release/include>" CACHE PATH "Clang include base on Windows")
        set(LLVM_LIB_DIR "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:$ENV{ProgramData}/LLVM/RelWithDebInfo/lib>$<$<CONFIG:Release>:$ENV{ProgramData}/LLVM/Release/lib>" CACHE PATH "Clang lib base on Windows")
    else()
        set(LLVM_INCLUDE_DIR "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:/usr/local/LLVM/RelWithDebInfo/include>$<$<CONFIG:Release>:/usr/local/LLVM/Release/include>" CACHE PATH "Clang include base on Unix")
        set(LLVM_LIB_DIR "$<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:/usr/local/LLVM/RelWithDebInfo/lib>$<$<CONFIG:Release>:/usr/local/LLVM/Release/lib>" CACHE PATH "Clang lib base on Unix")
    endif()
endif()

# End FIND PACKAGE and FETCH CONTENT **************************************************************
#**************************************************************************************************

# Target Creation *********************************************************************************
#**************************************************************************************************

file(GLOB_RECURSE Lib${TARGET_NAME}_SOURCE 
    "${CMAKE_CURRENT_SOURCE_DIR}/*/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/generated/src/*.cpp"
)

file(GLOB Lib${TARGET_NAME}_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)
list(APPEND Lib${TARGET_NAME}_DIRS ".")

if (${PROJECT_NAME}_DEBUG)
    message(STATUS "Lib${TARGET_NAME}_DIRS: ${Lib${TARGET_NAME}_DIRS}")
    message(STATUS "Lib${TARGET_NAME}_SOURCE: ${Lib${TARGET_NAME}_SOURCE}")
endif()

if (NOT DEFINED ${PROJECT_NAME}_BUILD)
    set(${PROJECT_NAME}_BUILD ON)
endif()

if (${PROJECT_NAME}_BUILD)
    if (Lib${TARGET_NAME}_SOURCE)
        if (${PROJECT_NAME}_DEBUG)
            message(STATUS "Creating target: Lib${TARGET_NAME} library")
        endif()

        add_library(Lib${TARGET_NAME} STATIC ${Lib${TARGET_NAME}_SOURCE})

        # Only add llvm-clang dependency if using an external build
        if (NOT USE_SYSTEM_CLANG)
            # Ensure Clang fetch completes before compiling
            add_dependencies(Lib${TARGET_NAME} llvm-install)

            # Add Clang headers and libraries for linking
            target_include_directories(Lib${TARGET_NAME} SYSTEM PUBLIC ${LLVM_INCLUDE_DIR})
            target_link_directories(Lib${TARGET_NAME} PUBLIC "${LLVM_LIB_DIR}")

            target_link_libraries(Lib${TARGET_NAME} PUBLIC 
            clangAnalysis
            #clangAnalysisFlowSensitive
            #clangAnalysisFlowSensitiveModels
            clangAPINotes
            #clangARCMigrate
            clangAST
            clangASTMatchers
            clangBasic
            #clangCodeGen
            #clangCrossTU
            #clangDependencyScanning
            #clangDirectoryWatcher
            clangDriver
            #clangDynamicASTMatchers
            clangEdit
            #clangExtractAPI
            #clangFormat
            clangFrontend
            #clangFrontendTool
            #clangHandleCXX
            #clangHandleLLVM
            #clangIndex
            #clangIndexSerialization
            #clangInstallAPI
            #clangInterpreter
            clangLex
            clangParse
            #clangRewrite
            #clangRewriteFrontend
            clangSema
            clangSerialization
            #clangStaticAnalyzerCheckers
            #clangStaticAnalyzerCore
            #clangStaticAnalyzerFrontend
            clangSupport
            clangTooling
            #clangToolingASTDiff
            #clangToolingCore
            #clangToolingInclusions
            #clangToolingInclusionsStdlib
            #clangToolingRefactoring
            #clangToolingSyntax
            #clangTransformer
            #libclang
            
            #LLVM-C
            #LLVMAggressiveInstCombine
            LLVMAnalysis
            LLVMAsmParser
            #LLVMAsmPrinter
            LLVMBinaryFormat
            LLVMBitReader
            LLVMBitstreamReader
            #LLVMBitWriter
            #LLVMCFGuard
            #LLVMCFIVerify
            #LLVMCodeGen
            #LLVMCodeGenData
            #LLVMCodeGenTypes
            LLVMCore
            #LLVMCoroutines
            #LLVMCoverage
            LLVMDebugInfoBTF
            LLVMDebugInfoCodeView
            LLVMDebuginfod
            LLVMDebugInfoDWARF
            LLVMDebugInfoGSYM
            LLVMDebugInfoLogicalView
            LLVMDebugInfoMSF
            LLVMDebugInfoPDB
            LLVMDemangle
            #LLVMDiff
            #LLVMDlltoolDriver
            #LLVMDWARFLinker
            #LLVMDWARFLinkerClassic
            #LLVMDWARFLinkerParallel
            #LLVMDWP
            #LLVMExecutionEngine
            #LLVMExegesis
            #LLVMExegesisX86
            #LLVMExtensions
            #LLVMFileCheck
            #LLVMFrontendDriver
            #LLVMFrontendHLSL
            LLVMFrontendOffloading
            #LLVMFrontendOpenACC
            LLVMFrontendOpenMP
            #LLVMFuzzerCLI
            #LLVMFuzzMutate
            #LLVMGlobalISel
            #LLVMHipStdPar
            #LLVMInstCombine
            #LLVMInstrumentation
            #LLVMInterfaceStub
            #LLVMInterpreter
            #LLVMipo
            #LLVMIRPrinter
            LLVMIRReader
            #LLVMJITLink
            #LLVMLibDriver
            #LLVMLineEditor
            #LLVMLinker
            #LLVMLTO
            LLVMMC
            #LLVMMCA
            #LLVMMCDisassembler
            LLVMMCParser
            #LLVMMIRParser
            #LLVMObjCARCOpts
            #LLVMObjCopy
            LLVMObject
            #LLVMObjectYAML
            #LLVMOptDriver
            LLVMOption
            #LLVMOrcDebugging
            #LLVMOrcJIT
            #LLVMOrcShared
            #LLVMOrcTargetProcess
            #LLVMPasses
            LLVMProfileData
            LLVMRemarks
            #LLVMRuntimeDyld
            #LLVMSandboxIR
            LLVMScalarOpts
            #LLVMSelectionDAG
            LLVMSupport
            #LLVMSymbolize
            #LLVMTableGen
            #LLVMTableGenBasic
            #LLVMTableGenCommon
            #LLVMTarget
            LLVMTargetParser
            LLVMTextAPI
            #LLVMTextAPIBinaryReader
            LLVMTransformUtils
            #LLVMVectorize
            LLVMWindowsDriver
            #LLVMWindowsManifest
            #LLVMXRay
            #LLVMX86AsmParser
            #LLVMX86CodeGen
            #LLVMX86Desc
            #LLVMX86Disassembler
            #LLVMX86Info
            #LLVMX86TargetMCA
            #LTO
            #Remarks
            )
        else()
            # Use system Clang's include and library paths
            target_include_directories(Lib${TARGET_NAME} SYSTEM PUBLIC ${Clang_INCLUDE_DIRS})
            target_link_libraries(Lib${TARGET_NAME} PUBLIC 
            clangAnalysis
            clangAnalysisFlowSensitive
            clangAnalysisFlowSensitiveModels
            clangAPINotes
            clangARCMigrate
            clangAST
            clangASTMatchers
            clangBasic
            clangCodeGen
            clangCrossTU
            clangDependencyScanning
            clangDirectoryWatcher
            clangDriver
            clangDynamicASTMatchers
            clangEdit
            clangExtractAPI
            clangFormat
            clangFrontend
            clangFrontendTool
            clangHandleCXX
            clangHandleLLVM
            clangIndex
            clangIndexSerialization
            clangInstallAPI
            clangInterpreter
            clangLex
            clangParse
            clangRewrite
            clangRewriteFrontend
            clangSema
            clangSerialization
            clangStaticAnalyzerCheckers
            clangStaticAnalyzerCore
            clangStaticAnalyzerFrontend
            clangSupport
            clangTooling
            clangToolingASTDiff
            clangToolingCore
            clangToolingInclusions
            clangToolingInclusionsStdlib
            clangToolingRefactoring
            clangToolingSyntax
            clangTransformer
            libclang
            
            LLVM-C
            LLVMAggressiveInstCombine
            LLVMAnalysis
            LLVMAsmParser
            LLVMAsmPrinter
            LLVMBinaryFormat
            LLVMBitReader
            LLVMBitstreamReader
            LLVMBitWriter
            LLVMCFGuard
            LLVMCFIVerify
            LLVMCodeGen
            LLVMCodeGenData
            LLVMCodeGenTypes
            LLVMCore
            LLVMCoroutines
            LLVMCoverage
            LLVMDebugInfoBTF
            LLVMDebugInfoCodeView
            LLVMDebuginfod
            LLVMDebugInfoDWARF
            LLVMDebugInfoGSYM
            LLVMDebugInfoLogicalView
            LLVMDebugInfoMSF
            LLVMDebugInfoPDB
            LLVMDemangle
            LLVMDiff
            LLVMDlltoolDriver
            LLVMDWARFLinker
            LLVMDWARFLinkerClassic
            LLVMDWARFLinkerParallel
            LLVMDWP
            LLVMExecutionEngine
            LLVMExegesis
            LLVMExegesisX86
            LLVMExtensions
            LLVMFileCheck
            LLVMFrontendDriver
            LLVMFrontendHLSL
            LLVMFrontendOffloading
            LLVMFrontendOpenACC
            LLVMFrontendOpenMP
            LLVMFuzzerCLI
            LLVMFuzzMutate
            LLVMGlobalISel
            LLVMHipStdPar
            LLVMInstCombine
            LLVMInstrumentation
            LLVMInterfaceStub
            LLVMInterpreter
            LLVMipo
            LLVMIRPrinter
            LLVMIRReader
            LLVMJITLink
            LLVMLibDriver
            LLVMLineEditor
            LLVMLinker
            LLVMLTO
            LLVMMC
            LLVMMCA
            LLVMMCDisassembler
            LLVMMCParser
            LLVMMIRParser
            LLVMObjCARCOpts
            LLVMObjCopy
            LLVMObject
            LLVMObjectYAML
            LLVMOptDriver
            LLVMOption
            LLVMOrcDebugging
            LLVMOrcJIT
            LLVMOrcShared
            LLVMOrcTargetProcess
            LLVMPasses
            LLVMProfileData
            LLVMRemarks
            LLVMRuntimeDyld
            LLVMSandboxIR
            LLVMScalarOpts
            LLVMSelectionDAG
            LLVMSupport
            LLVMSymbolize
            LLVMTableGen
            LLVMTableGenBasic
            LLVMTableGenCommon
            LLVMTarget
            LLVMTargetParser
            LLVMTextAPI
            LLVMTextAPIBinaryReader
            LLVMTransformUtils
            LLVMVectorize
            LLVMWindowsDriver
            LLVMWindowsManifest
            LLVMXRay
            LLVMX86AsmParser
            LLVMX86CodeGen
            LLVMX86Desc
            LLVMX86Disassembler
            LLVMX86Info
            LLVMX86TargetMCA
            LTO
            Remarks
            )
        endif()

        if(WIN32)
            target_link_libraries(Lib${TARGET_NAME} PUBLIC ntdll Version.lib)
        endif()

        add_link_options(/ignore:4099)

        if (${PROJECT_NAME}_DEBUG)
            message(STATUS "Adding include directory: ${CMAKE_SOURCE_DIR}/generated/include")
        endif()

        target_include_directories(Lib${TARGET_NAME} PUBLIC 
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/generated/include> 
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Lib${TARGET_NAME}/generated> # Used when installed
        )

        # Set up include directories
        foreach (dir ${Lib${TARGET_NAME}_DIRS})
            if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include")
                if (${PROJECT_NAME}_DEBUG)
                    message(STATUS "Adding include directory: ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include")
                endif()
                target_include_directories(Lib${TARGET_NAME} PUBLIC 
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir}/include> 
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Lib${TARGET_NAME}/${dir}> # Used when installed
                )
            endif()

            if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inl")
                if (${PROJECT_NAME}_DEBUG)
                    message(STATUS "Adding inl directory: ${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inl")
                endif()
                target_include_directories(Lib${TARGET_NAME} PUBLIC 
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${dir}/inl> 
                    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/Lib${TARGET_NAME}/${dir}> # Used when installed
                )
            endif()
        endforeach()

        set_target_properties(Lib${TARGET_NAME} PROPERTIES 
            VERSION ${PROJECT_VERSION} 
            SOVERSION ${PROJECT_VERSION_MAJOR}
        )

        if (${PROJECT_NAME}_DEBUG)
            message(STATUS "Creating target: ${TARGET_NAME} executable")
        endif()

        add_executable(${TARGET_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

        target_link_libraries(${TARGET_NAME} PRIVATE LibGenSerialize)

        set_target_properties(${TARGET_NAME} PROPERTIES 
            VERSION ${PROJECT_VERSION} 
            SOVERSION ${PROJECT_VERSION_MAJOR}
        )
# End Target Creation *****************************************************************************
#**************************************************************************************************

# Add Sub-directories *****************************************************************************
#**************************************************************************************************

add_subdirectory(StandardPlugins)

# End Add Sub-directories *************************************************************************
#**************************************************************************************************

# Installation and Packing Configuration **********************************************************
#**************************************************************************************************

	    # Install the targets
	    install(
		    TARGETS ${TARGET_NAME} Lib${TARGET_NAME} 
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

        #[[
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
        ]]

# End Custom Commands *****************************************************************************
#**************************************************************************************************

# Create Unit Test Groups *************************************************************************
#**************************************************************************************************
if (GEN_TOOLS_PACKAGE_BUILD_TESTS)
	if(${PROJECT_NAME}_DEBUG)
		message(STATUS "Building test suit for ${TARGET_NAME}")
	endif()

	set(${TARGET_NAME}_TEST_DIRS "")

	foreach(dir ${Lib${TARGET_NAME}_DIRS})
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