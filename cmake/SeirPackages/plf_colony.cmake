# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_plf_colony result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED" "" ${ARGN})
	_seir_provide_begin("plf_colony")
	set(version "abb0aa6525a3dae56aacf50899517f47e7036016") # 7.41
	seir_download("https://raw.githubusercontent.com/mattreecebentley/plf_colony/${version}/plf_colony.h"
		SHA256 "f584b8e47e851f68a08326a23246b0078a5596d96b69da4b1a61fbf1affb05c2"
		RESULT downloaded
		)
	set(install_dir ${SEIR_3RDPARTY_DIR}/plf_colony)
	if(downloaded OR NOT EXISTS ${install_dir})
		file(INSTALL ${SEIR_DOWNLOAD_DIR}/plf_colony.h DESTINATION ${install_dir}/include)
		file(INSTALL ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/plf_colony-config.cmake DESTINATION ${install_dir}/lib/cmake/plf_colony)
		message(STATUS "[SEIR] Provided plf_colony at ${install_dir}")
		if(arg_SET_UPDATED)
			set(${arg_SET_UPDATED} ON PARENT_SCOPE)
		endif()
	endif()
	_seir_provide_end_library("plf_colony")
endfunction()
