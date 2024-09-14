# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_embed ${SOURCES})
target_link_libraries(seir_embed PRIVATE Seir::data Seir::u8main fmt::fmt)
seir_target(seir_embed FOLDER utils/embed STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_embed)
