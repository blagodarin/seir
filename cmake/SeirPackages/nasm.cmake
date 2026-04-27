# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

# https://www.nasm.us/
function(seir_provide_nasm result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED" "" ${ARGN})
	_seir_provide_begin("nasm")
	set(version "3.01")
	set(package "nasm-${version}")
	if(WIN32)
		seir_download("https://www.nasm.us/pub/nasm/releasebuilds/${version}/win64/${package}-win64.zip"
			SHA256 "e0ba5157007abc7b1a65118a96657a961ddf55f7e3f632ee035366dfce039ca4"
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
