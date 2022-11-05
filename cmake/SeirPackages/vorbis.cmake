# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

include(${CMAKE_CURRENT_LIST_DIR}/ogg.cmake)

function(seir_provide_vorbis result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("vorbis")
	seir_provide_ogg(ogg_flag FLAG STATIC_RUNTIME ${arg_STATIC_RUNTIME} SET_UPDATED ogg_updated)
	set(version "1.3.7")
	set(package "libvorbis-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vorbis.patch)
	seir_download("http://downloads.xiph.org/releases/vorbis/${package}.tar.xz"
		SHA1 "0a2dd71a999656b8091506839e8007a61a8fda1f"
		EXTRACT_DIR ${package}
		PATCH ${patch}
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/vorbis)
	if(downloaded OR NOT EXISTS ${install_dir} OR ogg_updated)
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building Vorbis from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			${ogg_flag}
			-DCMAKE_POLICY_DEFAULT_CMP0074=NEW # find_package() uses <PackageName>_ROOT variables.
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			MSVC_WARNINGS 4244 4267 4305
			)
		message(STATUS "[SEIR] Provided Vorbis at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("Vorbis")
endfunction()
