# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_vma result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED;STATIC_RUNTIME" "" ${ARGN})
	_seir_provide_begin("vma")
	set(version "8b87b6cbf765d2b4d6cf66a06cc9004e11c096c4") # 3.0.1 lacks proper CMake support
	set(package "VulkanMemoryAllocator-${version}")
	seir_select(patch ${arg_STATIC_RUNTIME} ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/vma.patch)
	seir_download("https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/${version}.zip"
		NAME "${package}.zip"
		SHA1 "730e961a100507d2bbffbd5d955d5056705a4dec"
		EXTRACT_DIR "${package}"
		PATCH ${patch}
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/vma)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building VulkanMemoryAllocator from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir} OPTIONS
			-DCMAKE_POLICY_DEFAULT_CMP0091=NEW # MSVC runtime library flags are selected by an abstraction.
			)
		message(STATUS "[SEIR] Provided VulkanMemoryAllocator at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("VulkanMemoryAllocator")
endfunction()
