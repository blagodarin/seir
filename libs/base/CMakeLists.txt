# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

seir_folder(base)
set(HEADERS
	include/seir_base/allocator.hpp
	include/seir_base/buffer.hpp
	include/seir_base/endian.hpp
	include/seir_base/fixed.hpp
	include/seir_base/intrinsics.hpp
	include/seir_base/macros.hpp
	include/seir_base/pointer.hpp
	include/seir_base/rigid_vector.hpp
	include/seir_base/scope.hpp
	include/seir_base/static_vector.hpp
	include/seir_base/string_utils.hpp
	include/seir_base/utf8.hpp
	)
source_group("include" FILES ${HEADERS})
add_library(base INTERFACE ${HEADERS})
add_library(Seir::base ALIAS base)
target_include_directories(base INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
seir_install(base EXPORT SeirTargets)
install(DIRECTORY include/seir_base DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
