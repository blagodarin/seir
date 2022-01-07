# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_freetype result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("freetype")
	set(version "2.10.4") # 2.11.0 has an issue: https://gitlab.freedesktop.org/freetype/freetype/-/issues/1075
	set(package "freetype-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/freetype.patch)
	seir_download("https://downloads.sourceforge.net/project/freetype/freetype2/${version}/${package}.tar.gz"
		SHA1 "040d6a4be23708132c85ef9df837eb3f8a04c4ab"
		EXTRACT_DIR "${package}"
		PATCH ${patch}
		RESULT downloaded)
	set(install_dir ${SEIR_3RDPARTY_DIR}/freetype)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building Freetype from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			-DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=ON
			-DCMAKE_DISABLE_FIND_PACKAGE_BZip2=ON
			-DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=ON
			-DCMAKE_DISABLE_FIND_PACKAGE_PNG=ON
			-DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=ON
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			MSVC_WARNINGS 4244 4267 4312
			)
		message(STATUS "[SEIR] Provided Freetype at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("Freetype")
endfunction()
