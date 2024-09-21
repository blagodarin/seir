# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_pack ${SOURCES})
target_link_libraries(seir_pack PRIVATE Seir::compression Seir::data Seir::io Seir::serialization Seir::u8main)
seir_target(seir_pack FOLDER utils/pack STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_pack)
