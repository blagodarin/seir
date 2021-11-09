# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_nasm _output)
	cmake_parse_arguments(_arg "" "SET_UPDATED" "" ${ARGN})
	if(_arg_SET_UPDATED)
		set(${_arg_SET_UPDATED} OFF PARENT_SCOPE)
	endif()
	set(_version "2.15.05")
	set(_package "nasm-${_version}")
	if(WIN32)
		seir_download("https://www.nasm.us/pub/nasm/releasebuilds/${_version}/win64/${_package}-win64.zip"
			SHA1 "f3d25401783109ec999508af4dc967facf64971a"
			NAME "${_package}.zip"
			EXTRACT_DIR "${_package}"
			RESULT _downloaded)
		set(_path ${CMAKE_BINARY_DIR}/${_package}/nasm.exe)
		if(_downloaded)
			message(STATUS "[SEIR] Provided nasm at ${_path}")
			if(_arg_SET_UPDATED)
				set(${_arg_SET_UPDATED} ON PARENT_SCOPE)
			endif()
		endif()
		set(${_output} ${_path} PARENT_SCOPE)
	else()
		message(FATAL_ERROR "'nasm' is only available on Windows")
	endif()
endfunction()
