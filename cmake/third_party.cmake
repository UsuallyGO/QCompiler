
if(QCOMPILER_BUILD_THIRD_PARTY)
	message(STATUS "build third party")
	include(ExternalProject)

	set(GTEST_URL https://github.com/google/googletest)
	set(GTEST_TAG "master")

	set(GTEST_SOURCE_DIR ${THIRD_PARTY_SOURCE_DIR}/gtest)
	set(GTEST_BUILD_DIR ${THIRD_PARTY_BUILD_DIR}/gtest_build)
	set(GTEST_INSTALL_DIR ${THIRD_PARTY_INSTALL_DIR}/gtest)

	ExternalProject_Add(gtest
		PREFIX ${GTEST_SOURCE_DIR}
		GIT_REPOSITORY ${GTEST_URL}
		GIT_TAG ${GTEST_TAG}
		BINARY_DIR ${GTEST_BUILD_DIR}
		CMAKE_CACHE_ARGS
			-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
			-DCMAKE_INSTALL_PREFIX:STRING=${GTEST_INSTALL_DIR}
			-DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
			-DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
	)

	list(APPEND ALL_THIRD_INCLUDE_DIR ${GTEST_INSTALL_DIR}/include)
	list(APPEND ALL_THIRD_LIB_DIR ${GTEST_INSTALL_DIR}/lib)
	if(WIN32)
		set(GTEST_LINK_OBJS "libgtest.lib" "libgtest_main.lib")
	else()
		set(GTEST_LINK_OBJS "libgtest.a" "libgtest_main.a")
	endif()
	list(APPEND ALL_THIRD_LINK_OBJS ${GTEST_LINK_OBJS})
	
	#message(STATUS ${ALL_THIRD_LINK_OBJS})

	add_custom_target(gtest_create_header_dir
  		COMMAND ${CMAKE_COMMAND} -E make_directory ${GTEST_INSTALL_DIR}/include
  		DEPENDS gtest)

	add_custom_target(gtest_copy_headers_to_destination
  		COMMAND ${CMAKE_COMMAND} -E copy_directory ${THIRD_PARTY_BUILD_DIR}/include ${GTEST_INSTALL_DIR}/include
  		DEPENDS gtest_create_header_dir)

	add_custom_target(gtest_create_library_dir
  		COMMAND ${CMAKE_COMMAND} -E make_directory ${GTEST_INSTALL_DIR}/lib
  		DEPENDS gtest)

	add_custom_target(gtest_copy_libs_to_destination
  		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GTEST_LINK_OBJS} ${GTEST_INSTALL_DIR}/lib
  		DEPENDS gtest_create_library_dir)
elseif(QCOMPILER_BUILD_TESTS)
	#use default environment
	find_library(gtest_lib gtest)
	find_library(gtest_main_lib gtest_main)

	if(gtest_lib-FOUND OR gtest_main_lib-FOUND)
		message(STATUS "Gtest is found in this machine")
	else()
		message(STATUS "Choose to build test, but there is no gtest library."
							"Try to rebuild with QCOMIPLER_BUILD_THIRD_PARTY option.")
	endif()
endif()