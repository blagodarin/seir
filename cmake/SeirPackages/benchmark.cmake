# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_benchmark result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("benchmark")
	set(version "1.8.3")
	set(package "benchmark-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/benchmark.patch)
	seir_download("https://github.com/google/benchmark/archive/refs/tags/v${version}.tar.gz"
		NAME "${package}.tar.gz"
		SHA256 "6bc180a57d23d4d9515519f92b0c83d61b05b5bab188961f36ac7b06b0d9e9ce"
		EXTRACT_DIR "${package}"
		PATCH ${patch}
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/benchmark)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building benchmark from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
			-DBENCHMARK_ENABLE_GTEST_TESTS=OFF
			-DBENCHMARK_ENABLE_TESTING=OFF
			-DBENCHMARK_ENABLE_WERROR=OFF
			-DBENCHMARK_INSTALL_DOCS=OFF
			)
		message(STATUS "[SEIR] Provided benchmark at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("benchmark")
endfunction()
