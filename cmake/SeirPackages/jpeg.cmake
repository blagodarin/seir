# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

include(${CMAKE_CURRENT_LIST_DIR}/nasm.cmake)

# https://github.com/libjpeg-turbo/libjpeg-turbo/releases
function(seir_provide_jpeg result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("jpeg")
	seir_provide_nasm(nasm_flag FLAG SET_UPDATED nasm_updated)
	set(version "3.0.3")
	set(package "libjpeg-turbo-${version}")
	seir_download("https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/${version}/${package}.tar.gz"
		SHA256 "343e789069fc7afbcdfe44dbba7dbbf45afa98a15150e079a38e60e44578865d"
		EXTRACT_DIR "${package}"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/jpeg)
	if(_downloaded OR NOT EXISTS ${install_dir} OR nasm_updated)
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building JPEG from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} TARGET jpeg-static OPTIONS
			-DENABLE_SHARED=OFF
			-DREQUIRE_SIMD=ON
			-DWITH_ARITH_DEC=OFF
			-DWITH_ARITH_ENC=OFF
			-DWITH_CRT_DLL=OFF
			-DWITH_TURBOJPEG=OFF
			${nasm_flag}
			)
		file(INSTALL
			${build_dir}/jconfig.h
			${source_dir}/jerror.h
			${source_dir}/jmorecfg.h
			${source_dir}/jpeglib.h
			DESTINATION ${install_dir}/include
			)
		message(STATUS "[SEIR] Provided JPEG at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("JPEG")
endfunction()
