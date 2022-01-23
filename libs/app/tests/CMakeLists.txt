# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/app.cpp
	src/window.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_app_tests ${SOURCES})
target_link_libraries(seir_app_tests PRIVATE Seir::app Seir::base doctest::doctest_with_main)
seir_target(seir_app_tests FOLDER libs/app STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME app COMMAND seir_app_tests)
