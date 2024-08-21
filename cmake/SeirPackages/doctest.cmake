# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://github.com/doctest/doctest/releases
function(seir_provide_doctest result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("doctest")
	set(version "2.4.11")
	set(package "doctest-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME}
		${CMAKE_CURRENT_FUNCTION_LIST_DIR}/doctest_static.patch
		${CMAKE_CURRENT_FUNCTION_LIST_DIR}/doctest_shared.patch
		)
	seir_download("https://github.com/doctest/doctest/archive/refs/tags/v${version}.tar.gz"
		NAME "${package}.tar.gz"
		SHA256 "632ed2c05a7f53fa961381497bf8069093f0d6628c5f26286161fbd32a560186"
		EXTRACT_DIR "${package}"
		PATCH ${patch}
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/doctest)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building doctest from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DCMAKE_POLICY_DEFAULT_CMP0054=NEW # Only interpret if() arguments as variables or keywords when unquoted.
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			-DDOCTEST_WITH_TESTS=OFF
			)
		message(STATUS "[SEIR] Provided doctest at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("doctest")
endfunction()
