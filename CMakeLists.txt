# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.25)

project(Seir LANGUAGES CXX)

include(CMakeDependentOption)
include(FetchContent)
include(cmake/SeirPackages.cmake)

# Features.
option(SEIR_AUDIO_OGGVORBIS "Enable Ogg Vorbis audio support" OFF)
option(SEIR_AUDIO_WAV "Enable Waveform audio support" OFF)
option(SEIR_IMAGE_BMP "Enable BMP image support" OFF)
option(SEIR_IMAGE_DDS "Enable DDS image support" OFF)
option(SEIR_IMAGE_ICO "Enable ICO image support" OFF)
option(SEIR_IMAGE_JPEG "Enable JPEG image support" OFF)
option(SEIR_IMAGE_PNG "Enable PNG image support" OFF)
option(SEIR_IMAGE_TGA "Enable TGA image support" OFF)
option(SEIR_IMAGE_WEBP "Enable WebP image support" OFF)
cmake_dependent_option(SEIR_COMPRESSION_ZLIB "Enable zlib compression support" OFF "NOT SEIR_IMAGE_PNG" ON)
option(SEIR_COMPRESSION_ZSTD "Enable zstd compression support" OFF)

# Components.
option(SEIR_GUI "Build GUI library" ON)
# 4
cmake_dependent_option(SEIR_RENDERER "Build renderer library" ON "NOT SEIR_GUI" ON)
# 3
cmake_dependent_option(SEIR_APP "Build app library" ON "NOT SEIR_RENDERER" ON)
option(SEIR_UTILS "Build utils" ON)
# 2
cmake_dependent_option(SEIR_AUDIO "Build audio library" ON "NOT SEIR_AUDIO_OGGVORBIS;NOT SEIR_AUDIO_WAV" ON)
cmake_dependent_option(SEIR_PACKAGE "Build package library" ON "NOT SEIR_UTILS" ON)
cmake_dependent_option(SEIR_IMAGE "Build image library" ON "NOT SEIR_APP;NOT SEIR_IMAGE_BMP;NOT SEIR_IMAGE_DDS;NOT SEIR_IMAGE_ICO;NOT SEIR_IMAGE_JPEG;NOT SEIR_IMAGE_PNG;NOT SEIR_IMAGE_TGA;NOT SEIR_IMAGE_WEBP;NOT SEIR_GUI" ON)
cmake_dependent_option(SEIR_SERIALIZATION "Build serialization library" ON "NOT SEIR_UTILS" ON)
# 1
cmake_dependent_option(SEIR_COMPRESSION "Build compression library" ON "NOT SEIR_COMPRESSION_ZLIB;NOT SEIR_COMPRESSION_ZSTD;NOT SEIR_IMAGE;NOT SEIR_PACKAGE" ON)
cmake_dependent_option(SEIR_IO "Build IO library" ON "NOT SEIR_AUDIO;NOT SEIR_IMAGE;NOT SEIR_PACKAGE;NOT SEIR_SERIALIZATION" ON)
cmake_dependent_option(SEIR_GRAPHICS "Build graphics library" ON "NOT SEIR_APP" ON)
option(SEIR_SYNTH "Build synth library" ON)
cmake_dependent_option(SEIR_U8MAIN "Build u8main library" ON "NOT SEIR_UTILS" ON)
# 0
cmake_dependent_option(SEIR_BASE "Build base library" ON "NOT SEIR_COMPRESSION;NOT SEIR_IO;NOT SEIR_SYNTH;NOT SEIR_U8MAIN" ON)
cmake_dependent_option(SEIR_MATH "Build math library" ON "NOT SEIR_GRAPHICS" ON)

set(SEIR_RENDERER_BACKEND "Vulkan" CACHE STRING "Renderer backend")
set_property(CACHE SEIR_RENDERER_BACKEND PROPERTY STRINGS "" "Vulkan")

# Options.
option(SEIR_STATIC_RUNTIME "Build Seir with static MSVC runtime" OFF)
if(PROJECT_IS_TOP_LEVEL)
	option(SEIR_BENCHMARKS "Build Seir benchmarks" OFF)
	option(SEIR_EXAMPLES "Build Seir examples" OFF)
	option(SEIR_TESTS "Build Seir tests" OFF)
endif()

if(NOT PROJECT_IS_TOP_LEVEL AND NOT DEFINED CMAKE_FOLDER)
	set(CMAKE_FOLDER "Seir")
endif()

seir_defaults()

seir_provide_fmt(fmt_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_provide_plf_colony(plf_colony_ROOT)
find_package(fmt REQUIRED)
find_package(plf_colony REQUIRED)
find_package(Threads REQUIRED)
if(SEIR_AUDIO AND LINUX)
	find_package(ALSA REQUIRED)
endif()
if(SEIR_AUDIO_OGGVORBIS)
	# Ubuntu 22.10 Vorbis package doesn't provide CMake configs,
	# and find_package won't find Vorbis without CMake configs,
	# so we need a workaround if we aren't providing Vorbis either.
	if(LINUX AND "vorbis" IN_LIST SEIR_3RDPARTY_SKIP)
		find_package(PkgConfig REQUIRED)
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
if(SEIR_GUI)
	seir_provide_freetype(Freetype_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(Freetype REQUIRED)
endif()
if(SEIR_IMAGE_JPEG)
	seir_provide_jpeg(JPEG_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	set(CMAKE_FIND_FRAMEWORK NEVER) # https://gitlab.kitware.com/cmake/cmake/-/issues/18921
	find_package(JPEG REQUIRED)
endif()
if(SEIR_IMAGE_WEBP)
	seir_provide_webp(WebP_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(WebP REQUIRED)
endif()
if(SEIR_RENDERER AND SEIR_RENDERER_BACKEND STREQUAL "Vulkan")
	find_package(Vulkan REQUIRED)
	if(NOT TARGET Vulkan::glslc)
		message(SEND_ERROR "Could not find glslc, please install Vulkan SDK")
	endif()
	seir_provide_vma(VulkanMemoryAllocator_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(VulkanMemoryAllocator REQUIRED)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "^Clang|GNU$" AND SEIR_ARCHITECTURE MATCHES "^amd64|x86$")
	add_compile_options(-msse4.1)
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
	if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 18)
		add_compile_options(
			-Wno-switch-default # Warns on exhaustive switches.
			-Wno-unsafe-buffer-usage
			)
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
	add_compile_options(/MP /Wall /WX
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
		/wd5246 # the initialization of a subobject should be wrapped in braces
		# Warnings from the standard library headers:
		/wd4668 # https://developercommunity.visualstudio.com/t/Including-limitsh-triggers-error-__STDC/10514743
		)
endif()

if(SEIR_APP)
	add_subdirectory(libs/app)
endif()
if(SEIR_AUDIO)
	add_subdirectory(libs/audio)
endif()
if(SEIR_BASE)
	add_subdirectory(libs/base)
endif()
if(SEIR_COMPRESSION)
	add_subdirectory(libs/compression)
endif()
if(SEIR_GRAPHICS)
	add_subdirectory(libs/graphics)
endif()
if(SEIR_GUI)
	add_subdirectory(libs/gui)
endif()
if(SEIR_IMAGE)
	add_subdirectory(libs/image)
endif()
if(SEIR_IO)
	add_subdirectory(libs/io)
endif()
if(SEIR_MATH)
	add_subdirectory(libs/math)
endif()
if(SEIR_PACKAGE)
	add_subdirectory(libs/package)
endif()
if(SEIR_RENDERER)
	add_subdirectory(libs/renderer)
endif()
if(SEIR_SERIALIZATION)
	add_subdirectory(libs/serialization)
endif()
if(SEIR_SYNTH)
	add_subdirectory(libs/synth)
endif()
if(SEIR_U8MAIN)
	add_subdirectory(libs/u8main)
endif()

if(SEIR_UTILS)
	add_subdirectory(utils/embed)
	add_subdirectory(utils/pack)
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
	if(SEIR_BASE)
		add_subdirectory(libs/base/tests)
	endif()
	if(SEIR_COMPRESSION)
		add_subdirectory(libs/compression/tests)
	endif()
	if(SEIR_GRAPHICS)
		add_subdirectory(libs/graphics/tests)
	endif()
	if(SEIR_GUI)
		add_subdirectory(libs/gui/tests)
	endif()
	if(SEIR_IMAGE)
		add_subdirectory(libs/image/tests)
	endif()
	if(SEIR_IO)
		add_subdirectory(libs/io/tests)
	endif()
	if(SEIR_MATH)
		add_subdirectory(libs/math/tests)
	endif()
	if(SEIR_PACKAGE)
		add_subdirectory(libs/package/tests)
	endif()
	if(SEIR_SERIALIZATION)
		add_subdirectory(libs/serialization/tests)
	endif()
	if(SEIR_SYNTH)
		add_subdirectory(libs/synth/tests)
	endif()
	if(SEIR_U8MAIN)
		add_subdirectory(libs/u8main/tests)
	endif()
	if(SEIR_UTILS)
		add_subdirectory(utils/embed/tests)
	endif()
endif()

if(SEIR_BENCHMARKS)
	seir_provide_benchmark(benchmark_ROOT STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
	find_package(benchmark REQUIRED)
	if(SEIR_AUDIO)
		add_subdirectory(libs/audio/benchmarks)
	endif()
	if(SEIR_SYNTH)
		add_subdirectory(libs/synth/benchmarks)
	endif()
endif()

if(SEIR_EXAMPLES)
	if(SEIR_AUDIO)
		add_subdirectory(examples/audio_player)
	endif()
	if(SEIR_RENDERER AND SEIR_U8MAIN)
		add_subdirectory(examples/cube)
	endif()
	if(SEIR_GUI AND SEIR_U8MAIN)
		add_subdirectory(examples/gui)
	endif()
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/SeirConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/SeirConfig.cmake INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/SeirConfigVersion.cmake VERSION 0.0.0 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/SeirConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/SeirConfigVersion.cmake cmake/SeirUtils.cmake cmake/SeirPackages.cmake DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir)
install(DIRECTORY cmake/SeirPackages DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir)
install(EXPORT SeirTargets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Seir NAMESPACE Seir::)
