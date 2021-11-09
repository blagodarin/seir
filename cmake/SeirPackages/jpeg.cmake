# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

include(${CMAKE_CURRENT_LIST_DIR}/nasm.cmake)

function(seir_provide_jpeg _output)
	cmake_parse_arguments(_arg "" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	if(_arg_SET_UPDATED)
		set(${_arg_SET_UPDATED} OFF PARENT_SCOPE)
	endif()
	if(_arg_STATIC_RUNTIME)
		set(_patch ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/jpeg.patch)
	else()
		set(_patch "")
	endif()
	seir_provide_nasm(_nasm SET_UPDATED _nasm_updated)
	set(_version "2.1.1")
	set(_package "libjpeg-turbo-${_version}")
	seir_download("https://downloads.sourceforge.net/project/libjpeg-turbo/${_version}/${_package}.tar.gz"
		SHA1 "f9c3c17479f4fa2c76dba15125552fc9f6bfda80"
		EXTRACT_DIR "${_package}"
		PATCH ${_patch}
		RESULT _downloaded)
	set(_install_dir ${SEIR_PACKAGE_DIR}/jpeg)
	if(_downloaded OR NOT EXISTS ${_install_dir} OR _nasm_updated)
		set(_source_dir ${CMAKE_BINARY_DIR}/${_package})
		set(_build_dir ${_source_dir}-build)
		message(STATUS "[SEIR] Building JPEG from ${_source_dir}")
		_seir_cmake(${_source_dir} ${_build_dir} ${_install_dir} TARGET jpeg-static OPTIONS
			-DCMAKE_ASM_NASM_COMPILER=${_nasm}
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			-DENABLE_SHARED=OFF
			-DREQUIRE_SIMD=ON
			-DWITH_ARITH_DEC=OFF
			-DWITH_ARITH_ENC=OFF
			-DWITH_CRT_DLL=ON # Doesn't work, set to ON to prevent manual flag manipulation.
			-DWITH_TURBOJPEG=OFF
			)
		file(INSTALL
			${_build_dir}/jconfig.h
			${_source_dir}/jerror.h
			${_source_dir}/jmorecfg.h
			${_source_dir}/jpeglib.h
			DESTINATION ${_install_dir}/include)
		message(STATUS "[SEIR] Provided JPEG at ${_install_dir}")
		if(_arg_SET_UPDATED)
			set(${_arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	set(${_output} ${_install_dir} PARENT_SCOPE)
endfunction()
