// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_renderer/2d.hpp>

#include <seir_base/scope.hpp>
#include <seir_base/shared_ptr.hpp>
#include <seir_graphics/rectf.hpp>
#include <seir_renderer/mesh.hpp>
#include <seir_renderer/renderer.hpp>
#include "2d.hpp"
#include "pass.hpp"

#include <cassert>
#include <limits>
#include <vector>

namespace seir
{
	class Renderer2DImpl
	{
	public:
		struct Range
		{
			SharedPtr<Texture2D> _texture;
			uint32_t _indices = 0;
		};

		std::vector<Vertex2D> _vertexBuffer;
		std::vector<uint16_t> _indexBuffer;
		std::vector<Range> _ranges{ { nullptr, 0 } };
		RectF _textureRect{ SizeF{ 1, 1 } };
		Rgba32 _color = Rgba32::white();

		struct Batch
		{
			Vertex2D* _vertices = nullptr;
			uint16_t* _indices = nullptr;
			uint32_t _baseIndex = 0;
		};

		void clear() noexcept
		{
			_vertexBuffer.clear();
			_indexBuffer.clear();
			_ranges.clear();
			assert(_ranges.capacity() >= 1);
			_ranges.emplace_back(Renderer2DImpl::Range{ nullptr, 0 });
			_textureRect = RectF{ SizeF{ 1, 1 } };
			_color = Rgba32::white();
		}

		Batch prepareBatch(uint32_t vertexCount, uint32_t indexCount)
		{
			assert(vertexCount <= std::numeric_limits<uint16_t>::max());
			const auto nextIndex = static_cast<uint32_t>(_vertexBuffer.size());
			assert(nextIndex <= std::numeric_limits<uint16_t>::max() - vertexCount); // TODO: Support larger meshes.
			auto& currentRange = _ranges.back();
			const bool needDegenerate = currentRange._indices > 0;
			const auto addedIndices = indexCount + (needDegenerate ? 2 : 0);
			const auto vertexBufferSize = _vertexBuffer.size() + vertexCount;
			const auto indexBufferSize = _indexBuffer.size() + addedIndices;
			_vertexBuffer.reserve(vertexBufferSize);
			_indexBuffer.reserve(indexBufferSize);
			auto* const vertices = _vertexBuffer.data() + _vertexBuffer.size();
			auto* indices = _indexBuffer.data() + _indexBuffer.size();
			_vertexBuffer.resize(vertexBufferSize);
			_indexBuffer.resize(indexBufferSize);
			currentRange._indices += addedIndices;
			if (needDegenerate)
			{
				*indices++ = static_cast<uint16_t>(nextIndex - 1);
				*indices++ = static_cast<uint16_t>(nextIndex);
			}
			return { vertices, indices, nextIndex };
		}
	};

	Renderer2D::Renderer2D()
		: _impl{ std::make_unique<Renderer2DImpl>() }
	{
	}

	Renderer2D ::~Renderer2D() noexcept = default;

	void Renderer2D::addRect(const RectF& rect)
	{
		const auto batch = _impl->prepareBatch(4, 4);
		batch._vertices[0] = { rect.topLeft(), _impl->_textureRect.topLeft(), _impl->_color };
		batch._vertices[1] = { rect.bottomLeft(), _impl->_textureRect.bottomLeft(), _impl->_color };
		batch._vertices[2] = { rect.topRight(), _impl->_textureRect.topRight(), _impl->_color };
		batch._vertices[3] = { rect.bottomRight(), _impl->_textureRect.bottomRight(), _impl->_color };
		batch._indices[0] = static_cast<uint16_t>(batch._baseIndex);
		batch._indices[1] = static_cast<uint16_t>(batch._baseIndex + 1);
		batch._indices[2] = static_cast<uint16_t>(batch._baseIndex + 2);
		batch._indices[3] = static_cast<uint16_t>(batch._baseIndex + 3);
	}

	void Renderer2D::draw(RenderPass& pass)
	{
		if (_impl->_indexBuffer.empty())
			return;
		SEIR_FINALLY{ [this]() noexcept { _impl->clear(); } };
		static_cast<RenderPassImpl&>(pass).bind2DShaders();
		static_cast<RenderPassImpl&>(pass).update2DBuffers(_impl->_vertexBuffer, _impl->_indexBuffer);
		static_cast<RenderPassImpl&>(pass).begin2DRendering({
			.vertexAttributes{ VertexAttribute::f32x2, VertexAttribute::f32x2, VertexAttribute::un8x4 },
			.topology = MeshTopology::TriangleStrip,
			.indexType = MeshIndexType::u16,
		});
		for (uint32_t baseIndex = 0; const auto& range : _impl->_ranges)
		{
			if (!range._indices)
				continue;
			pass.bindTexture(range._texture);
			static_cast<RenderPassImpl&>(pass).draw2D(baseIndex, range._indices);
			baseIndex += range._indices;
		}
	}

	void Renderer2D::setColor(const Rgba32& color)
	{
		_impl->_color = color;
	}

	void Renderer2D::setTexture(const SharedPtr<Texture2D>& texture)
	{
		if (auto& currentRange = _impl->_ranges.back(); currentRange._texture == texture)
			return;
		else if (!currentRange._indices) // NOLINT(readability-else-after-return)
			currentRange._texture = texture;
		else
			_impl->_ranges.emplace_back(Renderer2DImpl::Range{ texture, 0 });
		_impl->_textureRect = RectF{ SizeF{ 1, 1 } };
	}

	void Renderer2D::setTextureRect(const RectF& rect)
	{
		const auto texture = _impl->_ranges.back()._texture.get();
		_impl->_textureRect = texture ? rect / texture->size() : RectF{ SizeF{ 1, 1 } };
	}
}
