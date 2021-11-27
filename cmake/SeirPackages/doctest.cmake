# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_doctest _output)
	cmake_parse_arguments(_arg "" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	if(_arg_SET_UPDATED)
		set(${_arg_SET_UPDATED} OFF PARENT_SCOPE)
	endif()
	if(_arg_STATIC_RUNTIME)
		set(_patch ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/doctest_static.patch)
	else()
		set(_patch ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/doctest_shared.patch)
	endif()
	set(_version "2.4.6")
	set(_package "doctest-${_version}")
	seir_download("https://github.com/onqtam/doctest/archive/refs/tags/${_version}.zip"
		NAME "${_package}.zip"
		SHA1 "2c228e0aa91d5cbbe8740eaf5b1a50e168171a47"
		EXTRACT_DIR "${_package}"
		PATCH ${_patch}
		RESULT _downloaded)
	set(_install_dir ${SEIR_PACKAGE_DIR}/doctest)
	if(_downloaded OR NOT EXISTS ${_install_dir})
		set(_source_dir ${CMAKE_BINARY_DIR}/${_package})
		set(_build_dir ${_source_dir}-build)
		message(STATUS "[SEIR] Building doctest from ${_source_dir}")
		_seir_cmake(${_source_dir} ${_build_dir} ${_install_dir} OPTIONS
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DCMAKE_POLICY_DEFAULT_CMP0054=NEW # Only interpret if() arguments as variables or keywords when unquoted.
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			-DDOCTEST_WITH_TESTS=OFF
			)
		message(STATUS "[SEIR] Provided doctest at ${_install_dir}")
		if(_arg_SET_UPDATED)
			set(${_arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	set(${_output} ${_install_dir} PARENT_SCOPE)
endfunction()
