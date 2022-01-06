# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_zstd _output)
	if("zstd" IN_LIST SEIR_3RDPARTY_SKIP)
		unset(${_output} PARENT_SCOPE)
		return()
	endif()
	cmake_parse_arguments(_arg "" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	if(_arg_SET_UPDATED)
		set(${_arg_SET_UPDATED} OFF PARENT_SCOPE)
	endif()
	set(_version "1.5.0")
	set(_package "zstd-${_version}")
	seir_download("https://github.com/facebook/zstd/releases/download/v${_version}/${_package}.tar.zst"
		SHA1 "7d7537e68325cf509aba0d1448cc03fce182457e"
		EXTRACT_DIR "${_package}"
		RESULT _downloaded)
	set(_install_dir ${SEIR_3RDPARTY_DIR}/zstd)
	if(_downloaded OR NOT EXISTS ${_install_dir})
		set(_source_dir ${CMAKE_BINARY_DIR}/${_package})
		set(_build_dir ${_source_dir}-build)
		message(STATUS "[SEIR] Building zstd from ${_source_dir}")
		_seir_cmake(${_source_dir}/build/cmake ${_build_dir} ${_install_dir} OPTIONS
			-DZSTD_BUILD_PROGRAMS=OFF
			-DZSTD_BUILD_SHARED=OFF
			-DZSTD_USE_STATIC_RUNTIME=${_arg_STATIC_RUNTIME}
			)
		message(STATUS "[SEIR] Provided zstd at ${_install_dir}")
		if(_arg_SET_UPDATED)
			set(${_arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	set(${_output} ${_install_dir} PARENT_SCOPE)
endfunction()
