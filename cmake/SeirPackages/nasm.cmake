# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_nasm result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED" "" ${ARGN})
	_seir_provide_begin("nasm")
	set(version "2.16.01")
	set(package "nasm-${version}")
	if(WIN32)
		seir_download("https://www.nasm.us/pub/nasm/releasebuilds/${version}/win64/${package}-win64.zip"
			SHA256 "029eed31faf0d2c5f95783294432cbea6c15bf633430f254bb3c1f195c67ca3a"
			NAME "${package}.zip"
			EXTRACT_DIR "${package}"
			RESULT downloaded
			)
		set(path ${CMAKE_BINARY_DIR}/${package}/nasm.exe)
		if(downloaded)
			message(STATUS "[SEIR] Provided nasm at ${path}")
			if(arg_SET_UPDATED)
				set(${arg_SET_UPDATED} ON PARENT_SCOPE)
			endif()
		endif()
		seir_select(${result} ${arg_FLAG} "-DCMAKE_ASM_NASM_COMPILER=${path}" ${path} PARENT_SCOPE)
	else()
		message(FATAL_ERROR "'nasm' is only available on Windows")
	endif()
endfunction()
