
cmake_minimum_required(VERSION 3.0.1)

set(QCOMPILER_NAME "QCompiler")
project(${QCOMPILER_NAME} C CXX)

option(QCOMPILER_BUILD_THIRD_PARTY "Build third party" OFF)
option(QCOMPILER_BUILD_TESTS "Build test files" OFF)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(THIRD_PARTY_SOURCE_DIR ${PROJECT_BINARY_DIR}/third_party_source)
set(THIRD_PARTY_BUILD_DIR ${PROJECT_BINARY_DIR}/third_party_build)
set(THIRD_PARTY_INSTALL_DIR ${PROJECT_BINARY_DIR}/third_party_install)

set(ALL_THIRD_INCLUDE_DIR "")
set(ALL_THIRD_LIB_DIR "")
set(ALL_THIRD_LINK_OBJS "")

include(functions)
include(third_party)

Get_all_cpp_files(cpp_files)
#Print_items(cpp_files)
Get_test_files(cpp_files test_files)
#Print_items(test_files)
Get_source_files(cpp_files source_files)
#Print_items(source_files)
Get_internal_header_files(source_internal_head_files)
#Print_items(source_internal_head_files)
Get_main_file(cpp_files main_source_file)

#Print_items(ALL_THIRD_INCLUDE_DIR)
#Print_items(ALL_THIRD_LIB_DIR)
#Print_items(ALL_THIRD_LINK_OBJS)

if(WIN32)
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT") 
#in debug model, cannot use /MTd, the executable file will abort, I don't know why 
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

if(QCOMPILER_BUILD_TESTS)
	#Print_items(ALL_THIRD_LINK_OBJS)
	#if(MSVC)
	#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
	#endif()
	#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/units)
	include_directories(${ALL_THIRD_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)

	file(GLOB_RECURSE head_files "${PROJECT_SOURCE_DIR}/include/*.h")
	add_library(qcompiler SHARED ${source_files} ${head_files} ${source_internal_head_files})
	#add_library(qcompiler STATIC ${source_files} ${head_files} ${source_internal_head_files})
	link_directories(${ALL_THIRD_LIB_DIR})
	target_link_libraries(qcompiler)

	foreach(one_test_file ${test_files}) 
		#remove the extend postfix from test file
		get_filename_component(test_exe ${one_test_file} NAME_WE) 
		add_executable(${test_exe} ${one_test_file})

		if(QCOMPILER_BUILD_THIRD_PARTY)
			target_link_libraries(${test_exe} ${GTEST_LINK_OBJS})
		endif()
		if(UNIX)
			#in linux, we need to link pthread for gtest manually 
			target_link_libraries(${test_exe} pthread)
		endif()
		target_link_libraries(${test_exe} qcompiler) 
	endforeach()

	set(EXPECT_GENERATOR "Visual Studio")
	if(CMAKE_GENERATOR STRGREATER EXPECT_GENERATOR)
		assign_source_group(${source_files})
		assign_source_group(${test_files})
		assign_source_group(${source_internal_head_files})
		assign_source_group(${head_files})
	endif()
else() 
	#build source files into executable files only
	include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)
	#get_property(dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)
	#foreach(dir ${dirs})
  	#	message(STATUS "dir='${dir}'")
	#endforeach()
	#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

	file(COPY ${PROJECT_SOURCE_DIR}/files/domain.rge DESTINATION ${PROJECT_BINARY_DIR})
	file(COPY ${PROJECT_SOURCE_DIR}/files/grammar.syn DESTINATION ${PROJECT_BINARY_DIR})
	file(COPY ${PROJECT_SOURCE_DIR}/files/sentence.stn DESTINATION ${PROJECT_BINARY_DIR})
	set(EXPECT_GENERATOR "Visual Studio")
	if(CMAKE_GENERATOR STRGREATER EXPECT_GENERATOR)
		file(GLOB_RECURSE head_files "${PROJECT_SOURCE_DIR}/include/*.h")
		file(GLOB_RECURSE rge_files "${PROJECT_BINARY_DIR}/*.rge")
		file(GLOB_RECURSE syn_files "${PROJECT_BINARY_DIR}/*.syn")
		file(GLOB_RECURSE stn_files "${PROJECT_BINARY_DIR}/*.stn")

		assign_source_group(${source_files})
		assign_source_group(main_source_file)
		assign_source_group(${source_internal_head_files})
		assign_source_group(${head_files})
		assign_source_group(${rge_files})
		assign_source_group(${syn_files})
		assign_source_group(${stn_files})
	endif()

	add_executable(qcompiler ${source_files} ${main_source_file} ${source_internal_head_files} ${head_files} ${rge_files} 
	${syn_files} ${stn_files})
endif()
