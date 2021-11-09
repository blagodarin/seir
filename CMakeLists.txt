# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

project(Seir LANGUAGES CXX)

include(cmake/SeirPackages.cmake)

seir_set_defaults()

option(SEIR_STATIC_RUNTIME "Build Seir with static runtime" OFF)
if(PROJECT_IS_TOP_LEVEL)
	option(SEIR_TESTS "Build Seir tests" OFF)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Werror -Weverything
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-Werror -Wall -Wextra
		-Wconversion
		-Wdisabled-optimization
		-Wduplicated-branches
		-Wduplicated-cond
		-Wlogical-op
		-Wmissing-declarations
		-Wmissing-include-dirs
		-Wnull-dereference
		-Wpedantic
		-Wredundant-decls
		-Wshadow
		-Wsign-conversion
		-Wundef
		-Wuninitialized
		-Wunsafe-loop-optimizations
		-Wunused-macros
		)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	add_compile_options(/Wall /WX
		/wd4514 # unreferenced inline function has been removed
		/wd4623 # default constructor was implicitly defined as deleted
		/wd4625 # copy constructor was implicitly defined as deleted
		/wd4626 # assignment operator was implicitly defined as deleted
		/wd4710 # function not inlined
		/wd4711 # function selected for automatic inline expansion
		/wd4738 # storing 32-bit float result in memory, possible loss of performance
		/wd4820 # 'bytes' bytes padding added after construct 'member_name'
		/wd5027 # move assignment operator was implicitly defined as deleted
		/wd5045 # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
		)
endif()

add_subdirectory(libs/base)

if(SEIR_TESTS)
	seir_provide_doctest(doctest_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(doctest REQUIRED)
	enable_testing()
	add_subdirectory(libs/base/tests)
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/SeirConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/SeirConfig.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/SeirConfigVersion.cmake VERSION 0.0.0 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SeirConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/SeirConfigVersion.cmake cmake/SeirUtils.cmake cmake/SeirPackages.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir)
install(DIRECTORY cmake/SeirPackages DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir)
install(EXPORT SeirTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir NAMESPACE Seir::)
