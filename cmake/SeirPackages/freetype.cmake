# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_freetype result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("freetype")
	set(version "2.13.2")
	set(package "freetype-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/freetype.patch)
	seir_download("https://downloads.sourceforge.net/project/freetype/freetype2/${version}/${package}.tar.xz"
		SHA256 "12991c4e55c506dd7f9b765933e62fd2be2e06d421505d7950a132e4f1bb484d"
		EXTRACT_DIR "${package}"
		PATCH ${patch}
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/freetype)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building Freetype from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			-DFT_DISABLE_BROTLI=ON
			-DFT_DISABLE_BZIP2=ON
			-DFT_DISABLE_HARFBUZZ=ON
			-DFT_DISABLE_PNG=ON
			-DFT_DISABLE_ZLIB=ON
			MSVC_WARNINGS 4244 4267
			)
		message(STATUS "[SEIR] Provided Freetype at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("Freetype")
endfunction()
