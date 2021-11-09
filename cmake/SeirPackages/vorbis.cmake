# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

include(${CMAKE_CURRENT_LIST_DIR}/ogg.cmake)

function(seir_provide_vorbis _output)
	cmake_parse_arguments(_arg "" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	if(_arg_SET_UPDATED)
		set(${_arg_SET_UPDATED} OFF PARENT_SCOPE)
	endif()
	if(_arg_STATIC_RUNTIME)
		set(_patch ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vorbis.patch)
	else()
		set(_patch "")
	endif()
	seir_provide_ogg(Ogg_ROOT STATIC_RUNTIME ${_arg_STATIC_RUNTIME} SET_UPDATED _ogg_updated)
	set(_version "1.3.7")
	set(_package "libvorbis-${_version}")
	seir_download("http://downloads.xiph.org/releases/vorbis/${_package}.tar.xz"
		SHA1 "0a2dd71a999656b8091506839e8007a61a8fda1f"
		EXTRACT_DIR ${_package}
		PATCH ${_patch}
		RESULT _downloaded)
	set(_install_dir ${SEIR_PACKAGE_DIR}/vorbis)
	if(_downloaded OR NOT EXISTS ${_install_dir} OR _ogg_updated)
		set(_source_dir ${CMAKE_BINARY_DIR}/${_package})
		set(_build_dir ${_source_dir}-build)
		message(STATUS "[SEIR] Building Vorbis from ${_source_dir}")
		_seir_cmake(${_source_dir} ${_build_dir} ${_install_dir} OPTIONS
			-DCMAKE_POLICY_DEFAULT_CMP0074=NEW # find_package() uses <PackageName>_ROOT variables.
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			-DOgg_ROOT=${Ogg_ROOT}
			MSVC_WARNINGS 4244 4267 4305
			)
		message(STATUS "[SEIR] Provided Vorbis at ${_install_dir}")
		if(_arg_SET_UPDATED)
			set(${_arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	set(${_output} ${_install_dir} PARENT_SCOPE)
endfunction()
