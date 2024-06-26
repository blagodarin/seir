# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

include(GNUInstallDirs)

set(SEIR_DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/_SeirDownloads CACHE PATH "Directory for Seir downloads")
mark_as_advanced(SEIR_DOWNLOAD_DIR)

function(seir_defaults)
	if(NOT DEFINED PROJECT_NAME)
		message(SEND_ERROR "PROJECT_NAME has not been defined")
		return()
	endif()
	if(PROJECT_IS_TOP_LEVEL)
		set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "gen")
		set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER ".cmake")
		set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	endif()
	set(CMAKE_CXX_STANDARD 23 PARENT_SCOPE)
	set(CMAKE_MAP_IMPORTED_CONFIG_DEBUG "Debug;;Release" PARENT_SCOPE) # "Release" may be the only configuration provided by the host system.
	set(CMAKE_MAP_IMPORTED_CONFIG_MINSIZEREL "MinSizeRel;Release;RelWithDebInfo;" PARENT_SCOPE)
	set(CMAKE_MAP_IMPORTED_CONFIG_RELEASE "Release;RelWithDebInfo;MinSizeRel;" PARENT_SCOPE)
	set(CMAKE_MAP_IMPORTED_CONFIG_RELWITHDEBINFO "RelWithDebInfo;Release;MinSizeRel;" PARENT_SCOPE)
	if(WIN32)
		set(CMAKE_DEBUG_POSTFIX "d" PARENT_SCOPE)
	endif()
	string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" architecture)
	if(architecture STREQUAL "x86_64")
		set(architecture "amd64")
	endif()
	if(architecture STREQUAL "amd64" AND CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(architecture "x86")
	endif()
	if(NOT SEIR_ARCHITECTURE STREQUAL architecture)
		set(SEIR_ARCHITECTURE ${architecture} CACHE INTERNAL "Target architecture")
		message(STATUS "[SEIR] Target architecture: ${SEIR_ARCHITECTURE}")
	endif()
	get_filename_component(root_dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}" DIRECTORY)
	set(SEIR_DATA_DIR "${root_dir}/data" PARENT_SCOPE)
endfunction()

function(seir_download _url)
	cmake_parse_arguments(_arg "" "EXTRACT_DIR;NAME;PATCH;RESULT;SHA1;SHA256" "" ${ARGN})
	if(DEFINED _arg_UNPARSED_ARGUMENTS)
		string(REPLACE ";" " " _unrecognized_arguments "${_arg_UNPARSED_ARGUMENTS}")
		message(SEND_ERROR "Unrecognized arguments: ${_unrecognized_arguments}")
		return()
	endif()
	if(_arg_RESULT)
		set(${_arg_RESULT} OFF PARENT_SCOPE)
	endif()
	if(_arg_NAME)
		set(_name ${_arg_NAME})
	else()
		string(REGEX REPLACE "^.*/([^/]+)$" "\\1" _name ${_url})
	endif()
	if(_arg_SHA256)
		set(_hash_algorithm SHA256)
		set(_hash_value "${_arg_SHA256}")
	elseif(_arg_SHA1)
		set(_hash_algorithm SHA1)
		set(_hash_value "${_arg_SHA1}")
	else()
		set(_hash_algorithm "")
		set(_hash_value "")
	endif()
	set(_path ${SEIR_DOWNLOAD_DIR}/${_name})
	set(_do_download ON)
	if(EXISTS ${_path})
		if(_hash_algorithm)
			file(${_hash_algorithm} ${_path} _actual_hash)
			if(NOT ${_hash_value} STREQUAL ${_actual_hash})
				message(STATUS "[SEIR] Removing ${_name} (${_hash_algorithm} hash mismatch)")
				file(REMOVE ${_path})
			else()
				set(_do_download OFF)
			endif()
		else()
			set(_do_download OFF)
		endif()
	endif()
	if(_do_download)
		message(STATUS "[SEIR] Downloading ${_name}")
		if(_hash_algorithm)
			file(DOWNLOAD ${_url} ${_path} LOG _log STATUS _download_status TLS_VERIFY ON EXPECTED_HASH ${_hash_algorithm}=${_hash_value})
		else()
			file(DOWNLOAD ${_url} ${_path} LOG _log STATUS _download_status TLS_VERIFY ON)
		endif()
		list(GET _download_status 0 _download_status_code)
		if(NOT _download_status_code EQUAL 0)
			message(SEND_ERROR "${_log}")
			return()
		endif()
	endif()
	if(_arg_EXTRACT_DIR)
		set(_extract_dir ${CMAKE_BINARY_DIR}/${_arg_EXTRACT_DIR})
		set(_do_extract ON)
		if(EXISTS ${_extract_dir})
			if(_do_download)
				message(STATUS "[SEIR] Removing ${_arg_EXTRACT_DIR}")
				file(REMOVE_RECURSE ${_extract_dir})
			else()
				set(_do_extract OFF)
			endif()
		endif()
		if(_do_extract)
			message(STATUS "[SEIR] Extracting ${_name}")
			execute_process(COMMAND ${CMAKE_COMMAND} -E tar x ${_path}
				WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
				COMMAND_ERROR_IS_FATAL ANY)
			if(_arg_PATCH)
				find_package(Git REQUIRED)
				message(STATUS "[SEIR] Patching ${_arg_EXTRACT_DIR}")
				execute_process(COMMAND ${GIT_EXECUTABLE} apply ${_arg_PATCH}
					WORKING_DIRECTORY ${_extract_dir}
					COMMAND_ERROR_IS_FATAL ANY)
			endif()
		endif()
	endif()
	if(_arg_RESULT AND (_do_download OR _do_extract))
		set(${_arg_RESULT} ON PARENT_SCOPE)
	endif()
endfunction()

function(seir_embed_spirv _target)
	if(NOT TARGET ${_target})
		message(SEND_ERROR "'${_target}' is not a target")
		return()
	endif()
	cmake_parse_arguments(_arg "" "" "FRAGMENT;VERTEX" ${ARGN})
	get_target_property(_binary_dir ${_target} BINARY_DIR)
	get_target_property(_source_dir ${_target} SOURCE_DIR)
	foreach(_source ${_arg_FRAGMENT})
		get_filename_component(_source_name ${_source} NAME)
		set(_output ${_source_name}.spirv.inc)
		add_custom_command(OUTPUT ${_binary_dir}/${_output}
			COMMAND Vulkan::glslc -fshader-stage=fragment -mfmt=num -O -o ${_output} ${_source_dir}/${_source}
			MAIN_DEPENDENCY ${_source}
			VERBATIM)
		target_sources(${_target} PRIVATE ${_binary_dir}/${_output})
		source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_source_dir}/${_source})
		source_group("gen" FILES ${_binary_dir}/${_output})
	endforeach()
	foreach(_source ${_arg_VERTEX})
		get_filename_component(_source_name ${_source} NAME)
		set(_output ${_source_name}.spirv.inc)
		add_custom_command(OUTPUT ${_binary_dir}/${_output}
			COMMAND Vulkan::glslc -fshader-stage=vertex -mfmt=num -O -o ${_output} ${_source_dir}/${_source}
			MAIN_DEPENDENCY ${_source}
			VERBATIM)
		target_sources(${_target} PRIVATE ${_binary_dir}/${_output})
		source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${_source_dir}/${_source})
		source_group("gen" FILES ${_binary_dir}/${_output})
	endforeach()
	target_include_directories(${_target} PRIVATE ${_binary_dir})
endfunction()

function(seir_install _target)
	if(NOT TARGET ${_target})
		message(SEND_ERROR "'${_target}' is not a target")
		return()
	endif()
	cmake_parse_arguments(_arg "" "EXPORT;INCLUDE_DIRECTORY" "" ${ARGN})
	if(DEFINED _arg_UNPARSED_ARGUMENTS)
		string(REPLACE ";" " " _unrecognized_arguments "${_arg_UNPARSED_ARGUMENTS}")
		message(SEND_ERROR "Unrecognized arguments: ${_unrecognized_arguments}")
		return()
	endif()
	if(DEFINED _arg_EXPORT)
		set(_keyword_EXPORT "EXPORT")
	else()
		set(_keyword_EXPORT "")
	endif()
	if(${_target} MATCHES "^seir_(.+)$")
		set_target_properties(${_target} PROPERTIES EXPORT_NAME ${CMAKE_MATCH_1})
	endif()
	install(TARGETS ${_target} ${_keyword_EXPORT} ${_arg_EXPORT}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
		)
	if(DEFINED _arg_INCLUDE_DIRECTORY)
		install(DIRECTORY ${_arg_INCLUDE_DIRECTORY} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
	endif()
	if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		get_target_property(_target_type ${_target} TYPE)
		get_target_property(_target_name ${_target} OUTPUT_NAME)
		get_target_property(_target_postfix ${_target} DEBUG_POSTFIX)
		if(NOT _target_name)
			set(_target_name ${_target})
		endif()
		if(NOT _target_postfix)
			set(_target_postfix "")
		endif()
		if(_target_type STREQUAL STATIC_LIBRARY)
			set_target_properties(${_target} PROPERTIES COMPILE_PDB_NAME "${_target_name}" COMPILE_PDB_NAME_DEBUG "${_target_name}${_target_postfix}")
			install(FILES "$<TARGET_FILE_DIR:${_target}>/$<$<NOT:$<CONFIG:Debug>>:$<TARGET_PROPERTY:${_target},COMPILE_PDB_NAME>>$<$<CONFIG:Debug>:$<TARGET_PROPERTY:${_target},COMPILE_PDB_NAME_DEBUG>>.pdb"
				DESTINATION ${CMAKE_INSTALL_LIBDIR}
				OPTIONAL)
		elseif(NOT _target_type STREQUAL INTERFACE_LIBRARY)
			set_target_properties(${_target} PROPERTIES PDB_NAME "${_target_name}" PDB_NAME_DEBUG "${_target_name}${_target_postfix}")
			install(FILES "$<TARGET_PDB_FILE:${_target}>"
				DESTINATION ${CMAKE_INSTALL_BINDIR}
				OPTIONAL)
		endif()
	endif()
endfunction()

# seir_select(<variable> <condition> <if_true> [<if_false>] [PARENT_SCOPE])
macro(seir_select variable condition if_true)
	if(${condition})
		if(${ARGC} GREATER_EQUAL 5)
			if(${ARGV4} STREQUAL "PARENT_SCOPE")
				set(${variable} ${if_true} PARENT_SCOPE)
			else()
				set(${variable} ${if_true})
			endif()
		elseif(${ARGC} GREATER_EQUAL 4)
			if(${ARGV3} STREQUAL "PARENT_SCOPE")
				set(${variable} ${if_true} PARENT_SCOPE)
			else()
				set(${variable} ${if_true})
			endif()
		else()
			set(${variable} ${if_true})
		endif()
	elseif(${ARGC} GREATER_EQUAL 4)
		if(${ARGC} GREATER_EQUAL 5)
			if(${ARGV4} STREQUAL "PARENT_SCOPE")
				set(${variable} ${ARGV3} PARENT_SCOPE)
			else()
				set(${variable} ${ARGV3})
			endif()
		elseif(${ARGV3} STREQUAL "PARENT_SCOPE")
			set(${variable} "" PARENT_SCOPE)
		else()
			set(${variable} ${ARGV3})
		endif()
	else()
		set(${variable} "")
	endif()
endmacro()

function(seir_target _target)
	if(NOT TARGET ${_target})
		message(SEND_ERROR "'${_target}' is not a target")
		return()
	endif()
	cmake_parse_arguments(_arg "" "FOLDER;STATIC_RUNTIME" "" ${ARGN})
	if(DEFINED _arg_UNPARSED_ARGUMENTS)
		string(REPLACE ";" " " _unrecognized_arguments "${_arg_UNPARSED_ARGUMENTS}")
		message(SEND_ERROR "Unrecognized arguments: ${_unrecognized_arguments}")
		return()
	endif()
	if(DEFINED _arg_FOLDER AND PROJECT_IS_TOP_LEVEL)
		set_target_properties(${_target} PROPERTIES FOLDER "${_arg_FOLDER}")
	endif()
	if(DEFINED _arg_STATIC_RUNTIME)
		set_target_properties(${_target} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${_arg_STATIC_RUNTIME}>>:DLL>")
	endif()
endfunction()
