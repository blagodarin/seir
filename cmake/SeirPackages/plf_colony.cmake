# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

function(seir_provide_plf_colony result)
	cmake_parse_arguments(arg "FLAG" "SET_UPDATED" "" ${ARGN})
	_seir_provide_begin("plf_colony")
	set(version "348174f0da2ea65b7a561b08412e81ce1f5e6161") # 7.06
	seir_download("https://raw.githubusercontent.com/mattreecebentley/plf_colony/${version}/plf_colony.h"
		SHA1 "90013e815f794c095221fcbe203780611744d56a"
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
