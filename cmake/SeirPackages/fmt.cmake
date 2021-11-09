# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_fmt _output)
	cmake_parse_arguments(_arg "" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	if(_arg_SET_UPDATED)
		set(${_arg_SET_UPDATED} OFF PARENT_SCOPE)
	endif()
	if(_arg_STATIC_RUNTIME)
		set(_patch ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/fmt.patch)
	else()
		set(_patch "")
	endif()
	set(_version "8.0.1")
	set(_package "fmt-${_version}")
	seir_download("https://github.com/fmtlib/fmt/releases/download/${_version}/${_package}.zip"
		SHA1 "68564915fb9e912f59eaa54575d878902f90295f"
		EXTRACT_DIR "${_package}"
		PATCH ${_patch}
		RESULT _downloaded)
	set(_install_dir ${SEIR_PACKAGE_DIR}/fmt)
	if(_downloaded OR NOT EXISTS ${_install_dir})
		set(_source_dir ${CMAKE_BINARY_DIR}/${_package})
		set(_build_dir ${_source_dir}-build)
		message(STATUS "[SEIR] Building fmt from ${_source_dir}")
		_seir_cmake(${_source_dir} ${_build_dir} ${_install_dir} OPTIONS
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DFMT_DOC=OFF
			-DFMT_OS=OFF
			-DFMT_TEST=OFF
			)
		message(STATUS "[SEIR] Provided fmt at ${_install_dir}")
		if(_arg_SET_UPDATED)
			set(${_arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	set(${_output} ${_install_dir} PARENT_SCOPE)
endfunction()
