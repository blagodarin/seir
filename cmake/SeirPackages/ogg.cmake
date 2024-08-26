# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://xiph.org/downloads/
function(seir_provide_ogg result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("ogg")
	set(version "1.3.5")
	set(package "libogg-${version}")
	seir_download("http://downloads.xiph.org/releases/ogg/${package}.tar.xz"
		SHA256 "c4d91be36fc8e54deae7575241e03f4211eb102afb3fc0775fbbc1b740016705"
		EXTRACT_DIR ${package}
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/ogg)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building Ogg from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} STATIC_RUNTIME ${arg_STATIC_RUNTIME} OPTIONS
			-DBUILD_TESTING=OFF
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
