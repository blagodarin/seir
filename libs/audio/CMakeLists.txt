# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_audio/decoder.hpp
	include/seir_audio/format.hpp
	include/seir_audio/player.hpp
	include/seir_audio/processing.hpp
	include/seir_audio/wav.hpp
	)
set(SOURCES
	src/backend.hpp
	src/decoder.cpp
	src/player.cpp
	src/processing.cpp
	)
if(SEIR_AUDIO_WAV)
	list(APPEND SOURCES src/decoder_wav.cpp src/decoder_wav.hpp)
endif()
if(WIN32)
	list(APPEND SOURCES src/backend_wasapi.cpp)
	set_property(SOURCE src/player.cpp APPEND PROPERTY COMPILE_OPTIONS
		/wd4355 # 'this': used in base member initializer list
		)
else()
	list(APPEND SOURCES src/backend_alsa.cpp)
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_audio STATIC ${HEADERS} ${SOURCES})
add_library(Seir::audio ALIAS seir_audio)
target_compile_definitions(seir_audio PUBLIC
	SEIR_AUDIO_WAV=$<BOOL:${SEIR_AUDIO_WAV}>
	)
target_include_directories(seir_audio PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_audio PUBLIC Seir::data Threads::Threads $<$<NOT:$<BOOL:${WIN32}>>:ALSA::ALSA>)
if(CMAKE_CXX_COMPILER_ID MATCHES "^Clang|GNU$" AND SEIR_ARCHITECTURE MATCHES "^amd64|x86$")
	target_compile_options(seir_audio PRIVATE -msse4.1) # NOTE: Audio benchmarks should have the same compile options as Seir::audio.
endif()
seir_target(seir_audio FOLDER audio STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_audio EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_audio)
