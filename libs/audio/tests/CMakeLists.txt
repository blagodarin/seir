# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

seir_folder(audio)
source_group("src" REGULAR_EXPRESSION ".*\\.(hpp|cpp)$")
add_executable(audio_tests
	src/processing.cpp
	)
target_link_libraries(audio_tests PRIVATE Seir::audio doctest::doctest_with_main)
set_target_properties(audio_tests PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>$<$<NOT:$<BOOL:${SEIR_STATIC_RUNTIME}>>:DLL>")
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options(audio_tests PRIVATE -Wno-padded)
endif()
add_test(NAME audio COMMAND audio_tests)
