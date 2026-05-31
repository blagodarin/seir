// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_io/buffer_writer.hpp>
#include <seir_io/reader.hpp>
#include <seir_math/vec.hpp>
#include <seir_model/mesh_data.hpp>

#include <cstdlib> // For std::strtof.
#include <cstring>
#include <charconv>
#include <vector>

namespace
{
	bool fromChars(const char* begin, const char* end, float& result) noexcept
	{
#ifdef _MSC_VER
		if (std::from_chars(begin, end, result).ec != std::errc{})
			return false;
#else
		result = std::strtof(std::string{ begin, end }.c_str(), nullptr);
#endif
		return true;
	}

	class LineParser
	{
	public:
		LineParser(const std::string& line) noexcept
		{
			_cursor = line.data();
			_end = line.data() + line.size();
		}

		bool consume(const char what) noexcept
		{
			if (*_cursor != what)
				return false;
			++_cursor;
			return true;
		}

		bool endOfLineOr(char lineComment) noexcept
		{
			while (*_cursor == ' ' || *_cursor == '\t')
				++_cursor;
			return _cursor == _end || *_cursor == lineComment;
		}

		bool isEnd() const noexcept
		{
			return _cursor == _end;
		}

		char nonSpace() noexcept
		{
			while (*_cursor == ' ' || *_cursor == '\t')
				++_cursor;
			const auto result = *_cursor;
			if (_cursor != _end)
				++_cursor;
			return result;
		}

		bool space() noexcept
		{
			if (*_cursor != ' ' && *_cursor != '\t')
				return false;
			do
				++_cursor;
			while (*_cursor == ' ' || *_cursor == '\t');
			return true;
		}

		bool value(uint32_t& value) noexcept
		{
			const auto begin = _cursor;
			if (*_cursor < '0' || *_cursor > '9')
				return false;
			do
				++_cursor;
			while (*_cursor >= '0' && *_cursor <= '9');
			return std::from_chars(begin, _cursor, value).ec == std::errc{};
		}

		bool value(float& value) noexcept
		{
			const auto begin = _cursor;
			if (*_cursor == '-')
				++_cursor;
			if (*_cursor < '0' || *_cursor > '9')
				return false;
			do
				++_cursor;
			while (*_cursor >= '0' && *_cursor <= '9');
			if (*_cursor == '.')
			{
				++_cursor;
				if (*_cursor < '0' || *_cursor > '9')
					return false;
				do
					++_cursor;
				while (*_cursor >= '0' && *_cursor <= '9');
			}
			return ::fromChars(begin, _cursor, value);
		}

	private:
		const char* _cursor = nullptr;
		const char* _end = nullptr;
	};

	class ObjParser
	{
	public:
		bool parseLine(const std::string& line)
		{
			switch (LineParser p{ line }; p.nonSpace())
			{
			case '\0':
				return p.isEnd();

			case '#':
				return true;

			case 'v':
				if (p.space())
				{
					seir::Vec3 v;
					if (p.value(v.x)
						&& p.space() && p.value(v.y)
						&& p.space() && p.value(v.z)
						&& p.endOfLineOr('#'))
					{
						_vertices.emplace_back(v);
						return true;
					}
				}
				else if (p.consume('t'))
				{
					seir::Vec2 v;
					if (p.space() && p.value(v.x)
						&& p.space() && p.value(v.y)
						&& p.endOfLineOr('#'))
					{
						_texCoords.emplace_back(v);
						return true;
					}
				}
				else if (p.consume('n'))
				{
					seir::Vec3 v;
					if (p.space() && p.value(v.x)
						&& p.space() && p.value(v.y)
						&& p.space() && p.value(v.z)
						&& p.endOfLineOr('#'))
					{
						_normals.emplace_back(v);
						return true;
					}
				}
				break;

			case 'f':
				return p.space() && parseReference(p)
					&& p.space() && parseReference(p)
					&& p.space() && parseReference(p)
					&& p.endOfLineOr('#');
			}
			return false;
		}

		seir::SharedPtr<seir::MeshData> finish()
		{
			const auto indexType = _indices.size() < std::numeric_limits<uint16_t>::max() // Max value may have special meaning.
				? seir::MeshIndexType::u16
				: seir::MeshIndexType::u32;
			seir::MeshFormat format{ { seir::VertexAttribute::f32x3 }, seir::MeshTopology::TriangleList, indexType };
			if (!_normals.empty())
				format.vertexAttributes.push_back(seir::VertexAttribute::f32x3);
			if (!_texCoords.empty())
				format.vertexAttributes.push_back(seir::VertexAttribute::f32x2);
			const auto indexBufferSize = _indices.size()
				* (indexType == seir::MeshIndexType::u16 ? sizeof(uint16_t) : sizeof(uint32_t));
			seir::Buffer indexBuffer{ indexBufferSize };
			if (indexType == seir::MeshIndexType::u16)
			{
				auto indexData = reinterpret_cast<uint16_t*>(indexBuffer.data());
				for (auto i = _indices.begin(); i != _indices.end(); ++i)
					*indexData++ = static_cast<uint16_t>(*i);
			}
			else
				std::memcpy(indexBuffer.data(), _indices.data(), indexBufferSize);
			return seir::makeShared<seir::MeshData>(format, std::move(_vertexBuffer), _vertexCount, std::move(indexBuffer), _indices.size());
		}

	private:
		bool parseReference(LineParser& parser)
		{
			uint32_t vertexIndex = 0;
			uint32_t texCoordIndex = 0;
			uint32_t normalIndex = 0;
			if (!parser.value(vertexIndex) || vertexIndex == 0 || vertexIndex > _vertices.size())
				return false;
			if (parser.consume('/'))
			{
				if (parser.value(texCoordIndex) && (texCoordIndex == 0 || texCoordIndex > _texCoords.size()))
					return false;
				if (parser.consume('/')
					&& (!parser.value(normalIndex) || normalIndex == 0 || normalIndex > _normals.size()))
					return false;
			}
			const std::tuple reference{ vertexIndex, texCoordIndex, normalIndex };
			if (const auto i = std::find(_references.cbegin(), _references.cend(), reference); i != _references.cend())
			{
				_indices.emplace_back(static_cast<uint32_t>(i - _references.cbegin()));
				return true;
			}
			_vertexBufferWriter.write(_vertices[vertexIndex - 1]);
			if (normalIndex > 0)
				_vertexBufferWriter.write(_normals[normalIndex - 1]);
			if (texCoordIndex > 0)
				_vertexBufferWriter.write(_texCoords[texCoordIndex - 1]);
			++_vertexCount;
			_indices.emplace_back(static_cast<uint32_t>(_references.size()));
			_references.emplace_back(reference);
			return true;
		}

	private:
		std::vector<seir::Vec3> _vertices;
		std::vector<seir::Vec2> _texCoords;
		std::vector<seir::Vec3> _normals;
		std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> _references;
		seir::Buffer _vertexBuffer;
		seir::BufferWriter _vertexBufferWriter{ _vertexBuffer };
		size_t _vertexCount = 0;
		std::vector<uint32_t> _indices;
	};
}

namespace seir
{
	SharedPtr<MeshData> MeshData::load(const SharedPtr<Blob>& blob)
	{
		Reader reader{ *blob };
		ObjParser parser;
		for (std::string line;;)
		{
			line = reader.readLine();
			if (line.empty())
				break;
			if (line.back() == '\n')
			{
				line.pop_back();
				if (!line.empty() && line.back() == '\r')
					line.pop_back();
			}
			if (!parser.parseLine(line))
				return {};
		}
		return parser.finish();
	}
}
