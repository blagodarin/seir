# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_zlib result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("zlib")
	set(version "1.2.11")
	set(package "zlib-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/zlib.patch)
	seir_download("https://zlib.net/${package}.tar.xz"
		SHA1 "e1cb0d5c92da8e9a8c2635dfa249c341dfd00322"
		EXTRACT_DIR "${package}"
		PATCH ${patch}
		RESULT downloaded)
	set(install_dir ${SEIR_3RDPARTY_DIR}/zlib)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building ZLIB from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} TARGET zlibstatic OPTIONS
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			MSVC_WARNINGS 4267)
		file(INSTALL
			${build_dir}/zconf.h
			${source_dir}/zlib.h
			DESTINATION ${install_dir}/include)
		message(STATUS "[SEIR] Provided ZLIB at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("ZLIB")
endfunction()
