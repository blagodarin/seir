# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

include(${CMAKE_CURRENT_LIST_DIR}/nasm.cmake)

# https://github.com/libjpeg-turbo/libjpeg-turbo/releases
function(seir_provide_jpeg result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("jpeg")
	if(WIN32)
		seir_provide_nasm(nasm_flag FLAG SET_UPDATED nasm_updated)
		seir_select(with_crt_dll ${arg_STATIC_RUNTIME} OFF ON)
		set(win32_options
			-DWITH_CRT_DLL=${with_crt_dll}
			${nasm_flag}
			)
	else()
		set(win32_options "")
	endif()
	set(version "3.1.4.1")
	set(package "libjpeg-turbo-${version}")
	seir_download("https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/${version}/${package}.tar.gz"
		SHA256 "ecae8008e2cc9ade2f2c1bb9d5e6d4fb73e7c433866a056bd82980741571a022"
		EXTRACT_DIR "${package}"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/jpeg)
	if(_downloaded OR NOT EXISTS ${install_dir} OR nasm_updated)
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building JPEG from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			-DENABLE_SHARED=OFF
			-DREQUIRE_SIMD=ON
			-DWITH_ARITH_DEC=OFF
			-DWITH_ARITH_ENC=OFF
			-DWITH_TESTS=OFF
			-DWITH_TOOLS=OFF
			-DWITH_TURBOJPEG=OFF
			${win32_options}
			)
		message(STATUS "[SEIR] Provided JPEG at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("JPEG")
endfunction()
