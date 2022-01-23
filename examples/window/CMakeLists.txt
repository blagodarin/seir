# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(window WIN32 ${SOURCES})
target_link_libraries(window PRIVATE Seir::app Seir::u8main)
seir_target(window FOLDER examples STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
