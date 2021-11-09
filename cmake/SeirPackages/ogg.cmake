# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_ogg _output)
	cmake_parse_arguments(_arg "" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	if(_arg_SET_UPDATED)
		set(${_arg_SET_UPDATED} OFF PARENT_SCOPE)
	endif()
	if(_arg_STATIC_RUNTIME)
		set(_patch ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ogg.patch)
	else()
		set(_patch "")
	endif()
	set(_version "1.3.5")
	set(_package "libogg-${_version}")
	seir_download("http://downloads.xiph.org/releases/ogg/${_package}.tar.xz"
		SHA1 "5a368421a636f7faa4c2f662857cb507dffd7c99"
		EXTRACT_DIR ${_package}
		PATCH ${_patch}
		RESULT _downloaded)
	set(_install_dir ${SEIR_PACKAGE_DIR}/ogg)
	if(_downloaded OR NOT EXISTS ${_install_dir})
		set(_source_dir ${CMAKE_BINARY_DIR}/${_package})
		set(_build_dir ${_source_dir}-build)
		message(STATUS "[SEIR] Building Ogg from ${_source_dir}")
		_seir_cmake(${_source_dir} ${_build_dir} ${_install_dir} OPTIONS
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			-DINSTALL_DOCS=OFF
			-DINSTALL_PKG_CONFIG_MODULE=OFF
			)
		message(STATUS "[SEIR] Provided Ogg at ${_install_dir}")
		if(_arg_SET_UPDATED)
			set(${_arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	set(${_output} ${_install_dir} PARENT_SCOPE)
endfunction()
