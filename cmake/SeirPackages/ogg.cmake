# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_ogg result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("ogg")
	set(version "1.3.5")
	set(package "libogg-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/ogg.patch)
	seir_download("http://downloads.xiph.org/releases/ogg/${package}.tar.xz"
		SHA1 "5a368421a636f7faa4c2f662857cb507dffd7c99"
		EXTRACT_DIR ${package}
		PATCH ${patch}
		RESULT downloaded)
	set(install_dir ${SEIR_3RDPARTY_DIR}/ogg)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building Ogg from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			-DINSTALL_DOCS=OFF
			-DINSTALL_PKG_CONFIG_MODULE=OFF
			)
		message(STATUS "[SEIR] Provided Ogg at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("Ogg")
endfunction()
