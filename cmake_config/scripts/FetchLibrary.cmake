include(ExternalProject)

# Variables
function(FetchLibrary LIB_NAME API_URL REPO_URL TAG_FILE EXTERNAL_DOWNLOAD_DIR)
	
	# Fetch the remote's latest tag
	execute_process(
		COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/cmake_config/scripts/fetch_remote_latest_tag.py ${API_URL} 
		OUTPUT_VARIABLE REMOTE_TAG  
		RESULT_VARIABLE FETCH_RESULT 
		OUTPUT_STRIP_TRAILING_WHITESPACE 
	)

	if(NOT FETCH_RESULT EQUAL 0)
		message(WARNING "Failed to fetch remote tag: ${REMOTE_TAG}")
	endif()

	if(FETCH_RESULT EQUAL 0)
		# Read local hash if it exists
		if(EXISTS "${TAG_FILE}")
			file(READ "${TAG_FILE}" LOCAL_TAG)
		else()
			set(LOCAL_TAG "")
		endif()

		# Compare tags and update is necessary
		if(NOT "${LOCAL_TAG}" STREQUAL "${REMOTE_TAG}")
			message(STATUS "Tags differ, downloading new version of ${LIB_NAME}...")

			# Download and unpack the external project binaries
			ExternalProject_Add(
				${LIB_NAME} 
				URL "${REPO_URL}" 
				DOWNLOAD_DIR ${EXTERNAL_DOWNLOAD_DIR} 
				PREFIX ${EXTERNAL_DOWNLOAD_DIR}/${LIB_NAME} 
				CONFIGURE_COMMAND "" 
				BUILD_COMMAND "" 
				INSTALL_COMMAND "" 
			)

			# Write new hash to file
			file(WRITE "${TAG_FILE}" "${REMOTE_TAG}")
		else()
			message(STATUS "${LIB_NAME} is up to date, no download necessary")
		endif()

		ExternalProject_Get_Property(${LIB_NAME} SOURCE_DIR)
		set(${LIB_NAME}_SOURCE_DIR ${SOURCE_DIR} PARENT_SCOPE)
	endif()
endfunction()