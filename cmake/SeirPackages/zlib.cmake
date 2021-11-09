# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_zlib _output)
	cmake_parse_arguments(_arg "" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	if(_arg_SET_UPDATED)
		set(${_arg_SET_UPDATED} OFF PARENT_SCOPE)
	endif()
	if(_arg_STATIC_RUNTIME)
		set(_patch ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/zlib.patch)
	else()
		set(_patch "")
	endif()
	set(_version "1.2.11")
	set(_package "zlib-${_version}")
	seir_download("https://zlib.net/${_package}.tar.xz"
		SHA1 "e1cb0d5c92da8e9a8c2635dfa249c341dfd00322"
		EXTRACT_DIR "${_package}"
		PATCH ${_patch}
		RESULT _downloaded)
	set(_install_dir ${SEIR_PACKAGE_DIR}/zlib)
	if(_downloaded OR NOT EXISTS ${_install_dir})
		set(_source_dir ${CMAKE_BINARY_DIR}/${_package})
		set(_build_dir ${_source_dir}-build)
		message(STATUS "[SEIR] Building ZLIB from ${_source_dir}")
		_seir_cmake(${_source_dir} ${_build_dir} ${_install_dir} TARGET zlibstatic OPTIONS
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			MSVC_WARNINGS 4267)
		file(INSTALL
			${_build_dir}/zconf.h
			${_source_dir}/zlib.h
			DESTINATION ${_install_dir}/include)
		message(STATUS "[SEIR] Provided ZLIB at ${_install_dir}")
		if(_arg_SET_UPDATED)
			set(${_arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	set(${_output} ${_install_dir} PARENT_SCOPE)
endfunction()
