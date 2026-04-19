# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://www.zlib.net/
function(seir_provide_zlib result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("zlib")
	set(version "1.3.2")
	set(package "zlib-${version}")
	seir_download("https://zlib.net/${package}.tar.xz"
		SHA256 "d7a0654783a4da529d1bb793b7ad9c3318020af77667bcae35f95d0e42a792f3"
		EXTRACT_DIR "${package}"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/zlib)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building ZLIB from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} STATIC_RUNTIME ${arg_STATIC_RUNTIME} OPTIONS
			-DZLIB_BUILD_SHARED=OFF
			-DZLIB_BUILD_TESTING=OFF
			)
		message(STATUS "[SEIR] Provided ZLIB at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("ZLIB")
endfunction()
