# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_plf_colony _output)
	if("plf_colony" IN_LIST SEIR_3RDPARTY_SKIP)
		unset(${_output} PARENT_SCOPE)
		return()
	endif()
	cmake_parse_arguments(_arg "" "SET_UPDATED" "" ${ARGN})
	if(_arg_SET_UPDATED)
		set(${_arg_SET_UPDATED} OFF PARENT_SCOPE)
	endif()
	set(_version "760b64bb4e969296c7a2e77ee3b5da235da3004c") # 6.32
	seir_download("https://raw.githubusercontent.com/mattreecebentley/plf_colony/${_version}/plf_colony.h"
		SHA1 "578d689d86da218f35598c7b8f31a9b32703198c"
		RESULT _downloaded)
	set(_install_dir ${SEIR_3RDPARTY_DIR}/plf_colony)
	if(_downloaded OR NOT EXISTS ${_install_dir})
		file(INSTALL ${SEIR_DOWNLOAD_DIR}/plf_colony.h DESTINATION ${_install_dir}/include)
		file(INSTALL ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/plf_colony-config.cmake DESTINATION ${_install_dir}/lib/cmake/plf_colony)
		message(STATUS "[SEIR] Provided plf_colony at ${_install_dir}")
		if(_arg_SET_UPDATED)
			set(${_arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	set(${_output} ${_install_dir} PARENT_SCOPE)
endfunction()
