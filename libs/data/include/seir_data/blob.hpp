// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <filesystem>
#include <memory>

namespace seir
{
	class Blob
	{
	public:
		[[nodiscard]] static std::shared_ptr<Blob> from(const void* data, size_t size);
		[[nodiscard]] static std::shared_ptr<Blob> from(const std::shared_ptr<Blob>&, size_t offset, size_t size);
		[[nodiscard]] static std::shared_ptr<Blob> from(const std::filesystem::path&);

		virtual ~Blob() noexcept = default;
		[[nodiscard]] const void* data() const noexcept { return _data; }
		[[nodiscard]] size_t size() const noexcept { return _size; }

	protected:
		const void* _data = nullptr;
		size_t _size = 0;
		constexpr Blob(const void* data, size_t size) noexcept
			: _data{ data }, _size{ size } {}
	};
}

inline std::shared_ptr<seir::Blob> seir::Blob::from(const void* data, size_t size)
{
	struct Wrapper final : seir::Blob
	{
		constexpr Wrapper(const void* data, size_t size) noexcept
			: Blob{ data, size } {}
	};
	return std::make_shared<Wrapper>(data, size);
}

inline std::shared_ptr<seir::Blob> seir::Blob::from(const std::shared_ptr<seir::Blob>& parent, size_t offset, size_t size)
{
	class Subrange : public seir::Blob
	{
	public:
		Subrange(const std::shared_ptr<Blob>& parent, size_t offset, size_t size)
			: Blob{ static_cast<const std::byte*>(parent->data()) + offset, size }, _parent{ parent } {}

	private:
		const std::shared_ptr<seir::Blob> _parent;
	};
	if (offset > parent->size())
		offset = parent->size();
	const auto maxSize = parent->size() - offset;
	return std::make_shared<Subrange>(parent, offset, size < maxSize ? size : maxSize);
}
