# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

include(${CMAKE_CURRENT_LIST_DIR}/nasm.cmake)

function(seir_provide_jpeg result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("jpeg")
	seir_provide_nasm(nasm_flag FLAG SET_UPDATED nasm_updated)
	set(version "2.1.4")
	set(package "libjpeg-turbo-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/jpeg.patch)
	seir_download("https://downloads.sourceforge.net/project/libjpeg-turbo/${version}/${package}.tar.gz"
		SHA1 "5a355c08caa326cef7c2a61e062edfe8dd02ac07"
		EXTRACT_DIR "${package}"
		PATCH ${patch}
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/jpeg)
	if(_downloaded OR NOT EXISTS ${install_dir} OR nasm_updated)
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building JPEG from ${source_dir}")
		seir_select(extra_options "${WIN32}" -DWITH_CRT_DLL=ON) # Doesn't work, set to ON to prevent manual flag manipulation.
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} TARGET jpeg-static OPTIONS
			${nasm_flag}
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			-DENABLE_SHARED=OFF
			-DREQUIRE_SIMD=ON
			-DWITH_ARITH_DEC=OFF
			-DWITH_ARITH_ENC=OFF
			-DWITH_TURBOJPEG=OFF
			${extra_options}
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
