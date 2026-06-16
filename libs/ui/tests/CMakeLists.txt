# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(SOURCES
	src/font.cpp
	src/margins.cpp
	)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_executable(seir_ui_tests ${SOURCES})
target_compile_definitions(seir_ui_tests PRIVATE SEIR_DATA_DIR="${SEIR_DATA_DIR}/")
target_link_libraries(seir_ui_tests PRIVATE Seir::io Seir::renderer Seir::ui doctest::doctest_with_main)
seir_target(seir_ui_tests FOLDER libs/ui STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
add_test(NAME ui COMMAND seir_ui_tests)
set_tests_properties(ui PROPERTIES LABELS renderer)
