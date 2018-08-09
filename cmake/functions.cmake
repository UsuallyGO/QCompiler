# Notice about that, in CMake, you'd better to use different names to arguments and parameters of functions, or
# you may cannot set PARENT_SCOPE varaibles.
# I think this is a bug more than a feature.

function(Check_file_postfix file_name postfix CHECK_RES)
	string(LENGTH ${file_name} file_name_length)
	string(LENGTH ${postfix} postfix_length)
	math(EXPR begin_pos ${file_name_length}-${postfix_length})
	
	if(${begin_pos} GREATER 0 OR ${begin_pos} EQUAL 0)
		string(SUBSTRING ${file_name} ${begin_pos} ${postfix_length} file_post)
		string(COMPARE EQUAL ${file_post} ${postfix} ${CHECK_RES})
	endif()
	
	set(${CHECK_RES} ${${CHECK_RES}} PARENT_SCOPE)
endfunction()

function(Get_all_cpp_files all_cpp_files)
	file(GLOB_RECURSE src_files "${PROJECT_SOURCE_DIR}/src/*.cpp")
	#file(GLOB main_file "${PROJECT_SOURCE_DIR}/src/main.cpp")
	#list(REMOVE_ITEM src_files ${main_file})
	set(${all_cpp_files} ${src_files} PARENT_SCOPE)
endfunction()

function(Get_test_files all_files all_test_files)
	foreach(one_file ${${all_files}})
		Check_file_postfix(${one_file} "_test.cpp" IS_TEST_FILE)
		if(IS_TEST_FILE)
			list(APPEND ${all_test_files} ${one_file})
		endif()
	endforeach()

	set(${all_test_files} ${${all_test_files}} PARENT_SCOPE)
endfunction()

function(Get_source_files all_files all_source_files)
	foreach(one_file ${${all_files}})
		Check_file_postfix(${one_file} "_test.cpp" IS_TEST_FILE)
		if(NOT IS_TEST_FILE)
			Check_file_postfix(${one_file} "main.cpp" IS_MAIN_FILE)
			if(NOT IS_MAIN_FILE)
				list(APPEND ${all_source_files} ${one_file})
			endif()
		endif()
	endforeach()

	set(${all_source_files} ${${all_source_files}} PARENT_SCOPE)
endfunction()

function(Get_internal_header_files all_internal_headers)
	file(GLOB_RECURSE internal_head_files "${PROJECT_SOURCE_DIR}/src/*.h")
	set(${all_internal_headers} ${internal_head_files} PARENT_SCOPE)
endfunction()

function(Get_main_file all_files main_file)
	foreach(one_file ${${all_files}})
		Check_file_postfix(${one_file} "main.cpp" IS_MAIN_FILE)
		if(IS_MAIN_FILE)
			set(${main_file} ${one_file} PARENT_SCOPE)
			break()
		endif()
	endforeach()
endfunction()

function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

function(Print_items all_items)
	message(STATUS "${all_items}")
	foreach(item ${${all_items}})
		message(STATUS "item:" ${item})
	endforeach()
endfunction()