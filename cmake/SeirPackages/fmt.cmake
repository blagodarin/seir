# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_fmt result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("fmt")
	set(version "8.1.1")
	set(package "fmt-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/fmt.patch)
	seir_download("https://github.com/fmtlib/fmt/releases/download/${version}/${package}.zip"
		SHA1 "07c3e8de31ad24d0dabaffaaa231cc7917a0fdd8"
		EXTRACT_DIR "${package}"
		PATCH ${patch}
		RESULT downloaded)
	set(install_dir ${SEIR_3RDPARTY_DIR}/fmt)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building fmt from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DFMT_DOC=OFF
			-DFMT_OS=OFF
			-DFMT_TEST=OFF
			)
		message(STATUS "[SEIR] Provided fmt at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("fmt")
endfunction()
