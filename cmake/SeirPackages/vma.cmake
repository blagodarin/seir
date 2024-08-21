# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/releases
function(seir_provide_vma result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED" "" ${ARGN})
	_seir_provide_begin("vma")
	set(version "3.1.0")
	set(package "VulkanMemoryAllocator-${version}")
	seir_download("https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v${version}.tar.gz"
		NAME "${package}.zip"
		SHA256 "ae134ecc37c55634f108e926f85d5d887b670360e77cd107affaf3a9539595f2"
		EXTRACT_DIR "${package}"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/vma)
	if(downloaded OR NOT EXISTS ${install_dir})
		set(source_dir ${CMAKE_BINARY_DIR}/${package})
		set(build_dir ${source_dir}-build)
		message(STATUS "[SEIR] Building VulkanMemoryAllocator from ${source_dir}")
		_seir_cmake(${source_dir} ${build_dir} ${install_dir})
		message(STATUS "[SEIR] Provided VulkanMemoryAllocator at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("VulkanMemoryAllocator")
endfunction()
