# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://github.com/fmtlib/fmt/releases
function(seir_provide_fmt result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("fmt")
	set(version "11.0.2")
	set(package "fmt-${version}")
	seir_download("https://github.com/fmtlib/fmt/releases/download/${version}/${package}.zip"
		SHA256 "40fc58bebcf38c759e11a7bd8fdc163507d2423ef5058bba7f26280c5b9c5465"
		EXTRACT_DIR "${package}"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/fmt)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building fmt from ${source_dir}")
		seir_select(static_runtime_option ${arg_STATIC_RUNTIME} -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>)
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DFMT_DOC=OFF
			-DFMT_OS=OFF
			-DFMT_TEST=OFF
			${static_runtime_option}
			)
		message(STATUS "[SEIR] Provided fmt at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("fmt")
endfunction()
