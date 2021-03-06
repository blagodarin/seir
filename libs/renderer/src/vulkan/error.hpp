// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "vulkan.hpp"

#include <cassert>

#ifndef NDEBUG
#	include <string>
#endif

namespace seir
{
	class VulkanContext;
	struct WindowDescriptor;

	struct VulkanError
	{
#ifndef NDEBUG
		const std::string_view _function;
		const std::string _message;
		VulkanError(std::string_view function, std::string&& message) noexcept
			: _function{ function.substr(0, function.find('(')) }, _message{ std::move(message) } {}
		VulkanError(std::string_view function, VkResult status) // NOLINT(cppcoreguidelines-pro-type-member-init)
			: VulkanError(function, std::to_string(status))
		{}
#endif
	};
}

#ifndef NDEBUG
#	define SEIR_VK_THROW(call, message) \
		throw seir::VulkanError(call, message)
#else
#	define SEIR_VK_THROW(call, message) \
		throw seir::VulkanError()
#endif

#define SEIR_VK(call) \
	do \
	{ \
		if (const auto status = (call); status != VK_SUCCESS) \
		{ \
			assert(!#call); \
			SEIR_VK_THROW(#call, std::to_string(status)); \
		} \
	} while (false)
