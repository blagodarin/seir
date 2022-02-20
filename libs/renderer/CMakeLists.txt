# This file is part of Seir.
# Copyright (C) Sergei Blagodarin.
# SPDX-License-Identifier: Apache-2.0

set(HEADERS
	include/seir_renderer/renderer.hpp
	)
if(SEIR_RENDERER STREQUAL "Dummy")
	set(SOURCES
		src/dummy/renderer.cpp
		)
elseif(SEIR_RENDERER STREQUAL "Vulkan")
	set(SOURCES
		src/vulkan/context.cpp
		src/vulkan/context.hpp
		src/vulkan/renderer.cpp
		src/vulkan/renderer.hpp
		src/vulkan/utils.cpp
		src/vulkan/utils.hpp
		src/vulkan/vulkan.hpp
		)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set_property(SOURCE src/vulkan/context.cpp APPEND PROPERTY COMPILE_OPTIONS
			/wd4191 # 'reinterpret_cast': unsafe conversion from 'PFN_vkVoidFunction' to '...'
			/wd5246 # the initialization of a subobject should be wrapped in braces
			)
	endif()
endif()
source_group("include" FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
add_library(seir_renderer STATIC ${HEADERS} ${SOURCES})
add_library(Seir::renderer ALIAS seir_renderer)
target_compile_definitions(seir_renderer
	PUBLIC
		SEIR_RENDERER_DUMMY=$<STREQUAL:${SEIR_RENDERER},Dummy>
		SEIR_RENDERER_VULKAN=$<STREQUAL:${SEIR_RENDERER},Vulkan>
	PRIVATE
		$<$<BOOL:${WIN32}>:VK_USE_PLATFORM_WIN32_KHR>
	)
target_include_directories(seir_renderer PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(seir_renderer PRIVATE Seir::app Seir::math
	$<$<CONFIG:Debug>:fmt::fmt>
	$<$<STREQUAL:${SEIR_RENDERER},Vulkan>:Vulkan::Vulkan>
	)
seir_target(seir_renderer FOLDER libs/renderer STATIC_RUNTIME ${SEIR_STATIC_RUNTIME})
if(SEIR_RENDERER STREQUAL "Vulkan")
	seir_embed_spirv(seir_renderer
		VERTEX src/vulkan/vertex_shader.glsl
		FRAGMENT src/vulkan/fragment_shader.glsl
		)
endif()
seir_install(seir_renderer EXPORT SeirTargets)
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/seir_renderer)
