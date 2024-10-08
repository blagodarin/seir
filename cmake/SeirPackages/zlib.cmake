# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://www.zlib.net/
function(seir_provide_zlib result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("zlib")
	set(version "1.3.1")
	set(package "zlib-${version}")
	seir_download("https://zlib.net/${package}.tar.xz"
		SHA256 "38ef96b8dfe510d42707d9c781877914792541133e1870841463bfa73f883e32"
		EXTRACT_DIR "${package}"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/zlib)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building ZLIB from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} TARGET zlibstatic STATIC_RUNTIME ${arg_STATIC_RUNTIME} OPTIONS
			-DZLIB_BUILD_EXAMPLES=OFF)
		file(INSTALL
			${build_dir}/zconf.h
			${source_dir}/zlib.h
			DESTINATION ${install_dir}/include
			)
		message(STATUS "[SEIR] Provided ZLIB at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("ZLIB")
endfunction()
