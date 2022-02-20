# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.21)

project(Seir LANGUAGES CXX)

include(CMakeDependentOption)
include(FetchContent)
include(cmake/SeirPackages.cmake)

# Features.
option(SEIR_AUDIO_AULOS "Enable Aulos audio support")
option(SEIR_AUDIO_OGGVORBIS "Enable Ogg Vorbis audio support")
option(SEIR_AUDIO_WAV "Enable Waveform audio support")
option(SEIR_COMPRESSION_ZLIB "Enable zlib compression support")
option(SEIR_COMPRESSION_ZSTD "Enable zstd compression support")
option(SEIR_IMAGE_BMP "Enable BMP image support")
option(SEIR_IMAGE_ICO "Enable ICO image support")
option(SEIR_IMAGE_JPEG "Enable JPEG image support")
option(SEIR_IMAGE_TGA "Enable TGA image support")

set(SEIR_RENDERER "" CACHE STRING "Renderer backend")
set_property(CACHE SEIR_RENDERER PROPERTY STRINGS "" "Dummy" "Vulkan")

# Components.
cmake_dependent_option(SEIR_APP "Build app library" OFF "NOT SEIR_RENDERER" ON)
cmake_dependent_option(SEIR_AUDIO "Build audio library" OFF "NOT SEIR_AUDIO_AULOS;NOT SEIR_AUDIO_OGGVORBIS;NOT SEIR_AUDIO_WAV" ON)
cmake_dependent_option(SEIR_DATA "Build data library" OFF "NOT SEIR_AUDIO;NOT SEIR_COMPRESSION_ZLIB;NOT SEIR_COMPRESSION_ZSTD" ON)
cmake_dependent_option(SEIR_IMAGE "Build image library" OFF "NOT SEIR_IMAGE_BMP;NOT SEIR_IMAGE_ICO;NOT SEIR_IMAGE_JPEG;NOT SEIR_IMAGE_TGA" ON)
cmake_dependent_option(SEIR_MATH "Build math library" OFF "NOT SEIR_RENDERER" ON)
option(SEIR_U8MAIN "Build u8main library")

# Options.
option(SEIR_STATIC_RUNTIME "Build Seir with static MSVC runtime")
if(PROJECT_IS_TOP_LEVEL)
	option(SEIR_BENCHMARKS "Build Seir benchmarks")
	option(SEIR_EXAMPLES "Build Seir examples")
	option(SEIR_TESTS "Build Seir tests")
endif()

if(NOT DEFINED CMAKE_FOLDER)
	seir_select(CMAKE_FOLDER ${PROJECT_IS_TOP_LEVEL} ".external" "Seir")
endif()

seir_defaults()

find_package(Threads REQUIRED)
if(NOT WIN32)
	find_package(PkgConfig REQUIRED)
endif()
if(SEIR_AUDIO AND NOT WIN32)
	find_package(ALSA REQUIRED)
endif()
if(SEIR_AUDIO_AULOS)
	set(AULOS_STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	FetchContent_Declare(Aulos GIT_REPOSITORY https://github.com/blagodarin/aulos.git GIT_TAG 519f59b687656a40e051550b14a105945d605b67)
	FetchContent_MakeAvailable(Aulos)
endif()
if(SEIR_AUDIO_OGGVORBIS)
	if("vorbis" IN_LIST SEIR_3RDPARTY_SKIP AND NOT WIN32)
		pkg_check_modules(vorbisfile REQUIRED IMPORTED_TARGET vorbisfile)
		add_library(Vorbis::vorbisfile ALIAS PkgConfig::vorbisfile)
	else()
		seir_provide_ogg(Ogg_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
		seir_provide_vorbis(Vorbis_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
		find_package(Vorbis REQUIRED)
	endif()
endif()
if(SEIR_COMPRESSION_ZLIB)
	seir_provide_zlib(ZLIB_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(ZLIB REQUIRED)
endif()
if(SEIR_COMPRESSION_ZSTD)
	seir_provide_zstd(zstd_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(zstd REQUIRED)
endif()
if(SEIR_IMAGE_JPEG)
	seir_provide_jpeg(JPEG_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	set(CMAKE_FIND_FRAMEWORK NEVER) # https://gitlab.kitware.com/cmake/cmake/-/issues/18921
	find_package(JPEG REQUIRED)
endif()
if(SEIR_RENDERER STREQUAL "Vulkan")
	find_package(Vulkan REQUIRED)
	if(NOT TARGET Vulkan::glslc)
		message(SEND_ERROR "Could not find glslc, please install Vulkan SDK")
	endif()
	seir_provide_fmt(fmt_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(fmt REQUIRED)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-Werror -Weverything
		-Wno-c++98-compat
		-Wno-c++98-compat-pedantic
		-Wno-ctad-maybe-unsupported
		-Wno-nested-anon-types
		-Wno-padded
		-Wno-tautological-type-limit-compare # https://reviews.llvm.org/D41512
		-Wno-weak-vtables
		)
	if(NOT APPLE)
		add_compile_options(-Wno-return-std-move-in-c++11)
	endif()
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
		/wd4464 # relative include path contains '..'
		/wd4514 # unreferenced inline function has been removed
		/wd4623 # default constructor was implicitly defined as deleted
		/wd4625 # copy constructor was implicitly defined as deleted
		/wd4626 # assignment operator was implicitly defined as deleted
		/wd4710 # function not inlined
		/wd4711 # function selected for automatic inline expansion
		/wd4738 # storing 32-bit float result in memory, possible loss of performance
		/wd4820 # 'bytes' bytes padding added after construct 'member_name'
		/wd5026 # move constructor was implicitly defined as deleted
		/wd5027 # move assignment operator was implicitly defined as deleted
		/wd5045 # Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
		)
endif()

if(SEIR_APP)
	add_subdirectory(libs/app)
endif()
if(SEIR_AUDIO)
	add_subdirectory(libs/audio)
endif()
if(TRUE)
	add_subdirectory(libs/base)
endif()
if(SEIR_DATA)
	add_subdirectory(libs/data)
endif()
if(SEIR_IMAGE)
	add_subdirectory(libs/image)
endif()
if(SEIR_MATH)
	add_subdirectory(libs/math)
endif()
if(SEIR_RENDERER)
	add_subdirectory(libs/renderer)
endif()
if(SEIR_U8MAIN)
	add_subdirectory(libs/u8main)
endif()

if(SEIR_TESTS)
	seir_provide_doctest(doctest_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(doctest REQUIRED)
	enable_testing()
	if(SEIR_APP)
		add_subdirectory(libs/app/tests)
	endif()
	if(SEIR_AUDIO)
		add_subdirectory(libs/audio/tests)
	endif()
	if(TRUE)
		add_subdirectory(libs/base/tests)
	endif()
	if(SEIR_DATA)
		add_subdirectory(libs/data/tests)
	endif()
	if(SEIR_IMAGE)
		add_subdirectory(libs/image/tests)
	endif()
	if(SEIR_MATH)
		add_subdirectory(libs/math/tests)
	endif()
	if(SEIR_U8MAIN)
		add_subdirectory(libs/u8main/tests)
	endif()
endif()

if(SEIR_BENCHMARKS)
	seir_provide_benchmark(benchmark_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(benchmark REQUIRED)
	if(SEIR_AUDIO)
		add_subdirectory(libs/audio/benchmarks)
	endif()
endif()

if(SEIR_EXAMPLES)
	if(SEIR_AUDIO)
		add_subdirectory(examples/audio_player)
	endif()
	if(SEIR_APP AND SEIR_U8MAIN)
		add_subdirectory(examples/window)
	endif()
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/SeirConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/SeirConfig.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/SeirConfigVersion.cmake VERSION 0.0.0 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SeirConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/SeirConfigVersion.cmake cmake/SeirUtils.cmake cmake/SeirPackages.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir)
install(DIRECTORY cmake/SeirPackages DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir)
install(EXPORT SeirTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir NAMESPACE Seir::)
