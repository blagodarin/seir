# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://github.com/doctest/doctest/releases
function(seir_provide_doctest result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("doctest")
	set(version "2.5.2")
	set(package "doctest-v${version}")
	seir_download("https://github.com/doctest/doctest/releases/download/v${version}/${package}.tar.gz"
		NAME "${package}.tar.gz"
		SHA256 "03635c39d59844f1550b9ffa56e3cbc79bddfed557e8465cd3692628aadba70a"
		EXTRACT_DIR "${package}"
		EXTRACT_INSIDE
		PATCH ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/doctest.patch
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/doctest)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building doctest from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} STATIC_RUNTIME ${arg_STATIC_RUNTIME} OPTIONS
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
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
