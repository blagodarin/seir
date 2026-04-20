# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/releases
function(seir_provide_vma result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED" "" ${ARGN})
	_seir_provide_begin("vma")
	set(version "3.3.0")
	set(package "VulkanMemoryAllocator-${version}")
	seir_download("https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/archive/refs/tags/v${version}.tar.gz"
		NAME "${package}.tar.gz"
		SHA256 "c4f6bbe6b5a45c2eb610ca9d231158e313086d5b1a40c9922cb42b597419b14e"
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
