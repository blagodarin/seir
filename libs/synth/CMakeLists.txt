# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_synth/common.hpp
	include/seir_synth/composition.hpp
	include/seir_synth/data.hpp
	include/seir_synth/format.hpp
	include/seir_synth/renderer.hpp
	include/seir_synth/shaper.hpp
	)
set(SOURCES
	src/acoustics.hpp
	src/composition.cpp
	src/composition.hpp
	src/data.cpp
	src/modulator.hpp
	src/oscillator.hpp
	src/period.hpp
	src/renderer.cpp
	src/tables.hpp
	src/voice.hpp
	src/wave.hpp
	)
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_synth STATIC ${HEADERS} ${SOURCES})
add_library(Seir::synth ALIAS seir_synth)
target_include_directories(seir_synth PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_synth PRIVATE Seir::base fmt::fmt)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(seir_synth PRIVATE -Wno-double-promotion -Wno-float-equal)
endif()
seir_target(seir_synth FOLDER libs/synth STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_synth EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_synth)
