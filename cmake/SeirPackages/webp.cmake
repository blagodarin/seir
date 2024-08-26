# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://developers.google.com/speed/webp/download
function(seir_provide_webp result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("webp")
	set(version "1.4.0")
	set(package "libwebp-${version}")
	seir_download("https://storage.googleapis.com/downloads.webmproject.org/releases/webp/${package}.tar.gz"
		SHA256 "61f873ec69e3be1b99535634340d5bde750b2e4447caa1db9f61be3fd49ab1e5"
		EXTRACT_DIR "${package}"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/webp)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building WebP from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} STATIC_RUNTIME ${arg_STATIC_RUNTIME} OPTIONS
			-DBUILD_SHARED_LIBS=OFF
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			-DWEBP_BUILD_ANIM_UTILS=OFF
			-DWEBP_BUILD_CWEBP=OFF
			-DWEBP_BUILD_DWEBP=OFF
			-DWEBP_BUILD_GIF2WEBP=OFF
			-DWEBP_BUILD_IMG2WEBP=OFF
			-DWEBP_BUILD_VWEBP=OFF
			-DWEBP_BUILD_WEBPINFO=OFF
			-DWEBP_BUILD_LIBWEBPMUX=OFF
			-DWEBP_BUILD_WEBPMUX=OFF
			-DWEBP_BUILD_EXTRAS=OFF
			)
		message(STATUS "[SEIR] Provided WebP at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("WebP")
endfunction()
