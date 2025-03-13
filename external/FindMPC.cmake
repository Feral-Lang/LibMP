# - Try to find the MPC libraries
# This module defines:
#  MPC_FOUND             - system has MPC lib
#  MPC_INCLUDE_DIR       - the MPC include directory
#  MPC_LIBRARIES_DIR     - directory where the MPC libraries are located
#  MPC_LIBRARIES         - Link these to use MPC

include(FindPackageHandleStandardArgs)

if(MPC_INCLUDE_DIR)
	set(MPC_in_cache TRUE)
else()
	set(MPC_in_cache FALSE)
endif()
if(NOT MPC_LIBRARIES)
	set(MPC_in_cache FALSE)
endif()

# Is it already configured?
if(NOT MPC_in_cache)
	find_path(MPC_INCLUDE_DIR
	          NAMES mpc.h
	          HINTS ENV MPC_INC_DIR
	                ENV MPC_DIR
	                $ENV{MPC_DIR}/include
	          PATH_SUFFIXES include
		  DOC "The directory containing the MPC header files")

	find_library(MPC_LIBRARY_RELEASE NAMES mpc
		     HINTS ENV MPC_LIB_DIR
	       	     	   ENV MPC_DIR
			   $ENV{MPC_DIR}/lib
		     PATH_SUFFIXES lib
		     DOC "Path to the Release MPC library")

	find_library(MPC_LIBRARY_DEBUG NAMES mpcd mpc
	  	     HINTS ENV MPC_LIB_DIR
	        	   ENV MPC_DIR
			   $ENV{MPC_DIR}/include
		     PATH_SUFFIXES lib
		     DOC "Path to the Debug MPC library")

	get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
	if(IS_MULTI_CONFIG)
		set(MPC_LIBRARIES debug ${MPC_LIBRARY_DEBUG} optimized ${MPC_LIBRARY_RELEASE})
	else()
		if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
			set(MPC_LIBRARIES ${MPC_LIBRARY_DEBUG})
		else()
			set(MPC_LIBRARIES ${MPC_LIBRARY_RELEASE})
		endif()
	endif()

	# Attempt to load a user-defined configuration for MPC if couldn't be found
	if (NOT MPC_INCLUDE_DIR OR NOT MPC_LIBRARIES)
		include(MPCConfig OPTIONAL)
	endif()
	if (MPC_INCLUDE_DIR)
		set(MPC_FOUND true)
	endif()
endif()