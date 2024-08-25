# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://github.com/facebook/zstd/releases
function(seir_provide_zstd result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("zstd")
	set(version "1.5.6")
	set(package "zstd-${version}")
	seir_download("https://github.com/facebook/zstd/releases/download/v${version}/${package}.tar.zst"
		SHA256 "4aa8dd1c1115c0fd6b6b66c35c7f6ce7bd58cc1dfd3e4f175b45b39e84b14352"
		EXTRACT_DIR "${package}"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/zstd)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building zstd from ${source_dir}")
		_seir_cmake(${source_dir}/build/cmake ${build_dir} ${install_dir} OPTIONS
			-DZSTD_BUILD_PROGRAMS=OFF
			-DZSTD_BUILD_SHARED=OFF
			-DZSTD_LEGACY_SUPPORT=OFF
			-DZSTD_USE_STATIC_RUNTIME=${arg_STATIC_RUNTIME}
			)
		message(STATUS "[SEIR] Provided zstd at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("zstd")
endfunction()
