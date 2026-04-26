# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://sourceforge.net/projects/freetype/files/freetype2/
function(seir_provide_freetype result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("freetype")
	set(version "2.14.3")
	set(package "freetype-${version}")
	seir_download("https://downloads.sourceforge.net/project/freetype/freetype2/${version}/${package}.tar.xz"
		SHA256 "36bc4f1cc413335368ee656c42afca65c5a3987e8768cc28cf11ba775e785a5f"
		EXTRACT_DIR "${package}"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/freetype)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building Freetype from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} STATIC_RUNTIME ${arg_STATIC_RUNTIME} OPTIONS
			-DFT_DISABLE_BROTLI=ON
			-DFT_DISABLE_BZIP2=ON
			-DFT_DISABLE_HARFBUZZ=ON
			-DFT_DISABLE_PNG=ON
			-DFT_DISABLE_ZLIB=ON
			)
		message(STATUS "[SEIR] Provided Freetype at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("Freetype")
endfunction()
