# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/main.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_synth_benchmarks ${SOURCES})
target_link_libraries(seir_synth_benchmarks PRIVATE Seir::synth)
seir_target(seir_synth_benchmarks FOLDER libs/synth STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
