# - Try to find the MPFR libraries
# This module defines:
#  MPFR_FOUND             - system has MPFR lib
#  MPFR_INCLUDE_DIR       - the MPFR include directory
#  MPFR_LIBRARIES_DIR     - directory where the MPFR libraries are located
#  MPFR_LIBRARIES         - Link these to use MPFR

include(FindPackageHandleStandardArgs)

if(MPFR_INCLUDE_DIR)
	set(MPFR_in_cache TRUE)
else()
	set(MPFR_in_cache FALSE)
endif()
if(NOT MPFR_LIBRARIES)
	set(MPFR_in_cache FALSE)
endif()

# Is it already configured?
if(NOT MPFR_in_cache)
	find_path(MPFR_INCLUDE_DIR
	          NAMES mpfr.h
	          HINTS ENV MPFR_INC_DIR
	                ENV MPFR_DIR
	                $ENV{MPFR_DIR}/include
	          PATH_SUFFIXES include
		  DOC "The directory containing the MPFR header files")

	find_library(MPFR_LIBRARY_RELEASE NAMES mpfr
		     HINTS ENV MPFR_LIB_DIR
	       	     	   ENV MPFR_DIR
			   $ENV{MPFR_DIR}/lib
		     PATH_SUFFIXES lib
		     DOC "Path to the Release MPFR library")

	find_library(MPFR_LIBRARY_DEBUG NAMES mpfrd mpfr
	  	     HINTS ENV MPFR_LIB_DIR
	        	   ENV MPFR_DIR
			   $ENV{MPFR_DIR}/include
		     PATH_SUFFIXES lib
		     DOC "Path to the Debug MPFR library")

	get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
	if(IS_MULTI_CONFIG)
		set(MPFR_LIBRARIES debug ${MPFR_LIBRARY_DEBUG} optimized ${MPFR_LIBRARY_RELEASE})
	else()
		if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
			set(MPFR_LIBRARIES ${MPFR_LIBRARY_DEBUG})
		else()
			set(MPFR_LIBRARIES ${MPFR_LIBRARY_RELEASE})
		endif()
	endif()

	# Attempt to load a user-defined configuration for MPFR if couldn't be found
	if (NOT MPFR_INCLUDE_DIR OR NOT MPFR_LIBRARIES)
		include(MPFRConfig OPTIONAL)
	endif()
	if (MPFR_INCLUDE_DIR)
		set(MPFR_FOUND true)
		include_directories(${MPFR_INCLUDE_DIR})
	endif()
endif()