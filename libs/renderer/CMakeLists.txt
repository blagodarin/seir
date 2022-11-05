# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_renderer/renderer.hpp
	)
set(SOURCES
	src/renderer.cpp
	)
if(SEIR_RENDERER STREQUAL "Dummy")
	list(APPEND SOURCES
		src/dummy/renderer.cpp
		)
elseif(SEIR_RENDERER STREQUAL "Vulkan")
	list(APPEND SOURCES
		src/vulkan/commands.cpp
		src/vulkan/commands.hpp
		src/vulkan/context.cpp
		src/vulkan/context.hpp
		src/vulkan/descriptors.cpp
		src/vulkan/descriptors.hpp
		src/vulkan/error.hpp
		src/vulkan/options.hpp
		src/vulkan/pipeline.cpp
		src/vulkan/pipeline.hpp
		src/vulkan/renderer.cpp
		src/vulkan/renderer.hpp
		src/vulkan/utils.cpp
		src/vulkan/utils.hpp
		src/vulkan/vulkan.hpp
		)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set_property(SOURCE src/vulkan/context.cpp APPEND PROPERTY COMPILE_OPTIONS
			/wd4191 # 'reinterpret_cast': unsafe conversion from 'PFN_vkVoidFunction' to '...'
			)
		set_property(SOURCE src/vulkan/renderer.cpp APPEND PROPERTY COMPILE_OPTIONS
			/wd5246 # the initialization of a subobject should be wrapped in braces
			)
	endif()
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_renderer STATIC ${HEADERS} ${SOURCES})
add_library(Seir::renderer ALIAS seir_renderer)
target_compile_definitions(seir_renderer PUBLIC
	SEIR_RENDERER_DUMMY=$<STREQUAL:${SEIR_RENDERER},Dummy>
	SEIR_RENDERER_VULKAN=$<STREQUAL:${SEIR_RENDERER},Vulkan>
	)
target_include_directories(seir_renderer PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_renderer PRIVATE Seir::app Seir::image Seir::math)
if(SEIR_RENDERER STREQUAL "Vulkan")
	target_compile_definitions(seir_renderer PRIVATE $<$<BOOL:${WIN32}>:VK_USE_PLATFORM_WIN32_KHR>)
	target_link_libraries(seir_renderer PRIVATE Vulkan::Vulkan $<$<CONFIG:Debug>:fmt::fmt>)
	seir_embed_spirv(seir_renderer
		VERTEX src/vulkan/vertex_shader.glsl
		FRAGMENT src/vulkan/fragment_shader.glsl
		)
endif()
seir_target(seir_renderer FOLDER libs/renderer STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
seir_install(seir_renderer EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_renderer)
