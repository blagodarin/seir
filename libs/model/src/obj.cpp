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
		LineParser(std::string& line)
		{
			assert(!line.empty());
			if (line.back() == '\n')
			{
				line.pop_back();
				if (!line.empty() && line.back() == '\r')
					line.pop_back();
			}
			_cursor = line.data();
			_end = line.data() + line.size();
		}

		void advance() noexcept
		{
			assert(_cursor != _end);
			++_cursor;
		}

		void skipSpaces() noexcept
		{
			while (*_cursor == ' ' || *_cursor == '\t')
				++_cursor;
		}

		bool tryConsume(const char what) noexcept
		{
			if (*_cursor != what)
				return false;
			++_cursor;
			return true;
		}

		bool tryConsumeSpaces() noexcept
		{
			if (*_cursor != ' ' && *_cursor != '\t')
				return false;
			do
				++_cursor;
			while (*_cursor == ' ' || *_cursor == '\t');
			return true;
		}

		bool tryParseFloat(float& value) noexcept
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

		bool tryParseUint32(uint32_t& value) noexcept
		{
			const auto begin = _cursor;
			if (*_cursor < '0' || *_cursor > '9')
				return false;
			do
				++_cursor;
			while (*_cursor >= '0' && *_cursor <= '9');
			return std::from_chars(begin, _cursor, value).ec == std::errc{};
		}

		char current() const noexcept
		{
			return *_cursor;
		}

	private:
		const char* _cursor = nullptr;
		const char* _end = nullptr;
	};

	class ObjParser
	{
	public:
		bool parseLine(std::string_view line)
		{
			_currentLine = line;
			LineParser parser{ _currentLine };
			parser.skipSpaces();
			switch (parser.current())
			{
			case '\0':
			case '#':
				break;

			case 'v':
				parser.advance();
				if (parser.tryConsumeSpaces())
				{
					seir::Vec3 v;
					if (!parser.tryParseFloat(v.x)
						|| !parser.tryConsumeSpaces()
						|| !parser.tryParseFloat(v.y)
						|| !parser.tryConsumeSpaces()
						|| !parser.tryParseFloat(v.z))
						return false;
					parser.skipSpaces();
					if (const auto current = parser.current(); current != '\0' && current != '#')
						return false;
					_vertices.emplace_back(v);
				}
				else if (parser.tryConsume('t'))
				{
					if (!parser.tryConsumeSpaces())
						return false;
					seir::Vec2 v;
					if (!parser.tryParseFloat(v.x)
						|| !parser.tryConsumeSpaces()
						|| !parser.tryParseFloat(v.y))
						return false;
					parser.skipSpaces();
					if (const auto current = parser.current(); current != '\0' && current != '#')
						return false;
					_texCoords.emplace_back(v);
				}
				else if (parser.tryConsume('n'))
				{
					if (!parser.tryConsumeSpaces())
						return false;
					seir::Vec3 v;
					if (!parser.tryParseFloat(v.x)
						|| !parser.tryConsumeSpaces()
						|| !parser.tryParseFloat(v.y)
						|| !parser.tryConsumeSpaces()
						|| !parser.tryParseFloat(v.z))
						return false;
					parser.skipSpaces();
					if (const auto current = parser.current(); current != '\0' && current != '#')
						return false;
					_normals.emplace_back(v);
				}
				else
					return false;
				break;

			case 'f':
				parser.advance();
				if (!parser.tryConsumeSpaces()
					|| !parseReference(parser)
					|| !parser.tryConsumeSpaces()
					|| !parseReference(parser)
					|| !parser.tryConsumeSpaces()
					|| !parseReference(parser))
					return false;
				parser.skipSpaces();
				if (const auto current = parser.current(); current != '\0' && current != '#')
					return false;
				break;

			default:
				return false;
			}
			return true;
		}

		seir::MeshData finish()
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
			return { format, std::move(_vertexBuffer), _vertexCount, std::move(indexBuffer), _indices.size() };
		}

	private:
		bool parseReference(LineParser& parser)
		{
			uint32_t vertexIndex = 0;
			uint32_t texCoordIndex = 0;
			uint32_t normalIndex = 0;
			if (!parser.tryParseUint32(vertexIndex) || vertexIndex == 0 || vertexIndex > _vertices.size())
				return false;
			if (parser.tryConsume('/'))
			{
				if (parser.tryParseUint32(texCoordIndex) && (texCoordIndex == 0 || texCoordIndex > _texCoords.size()))
					return false;
				if (parser.tryConsume('/')
					&& (!parser.tryParseUint32(normalIndex) || normalIndex == 0 || normalIndex > _normals.size()))
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
		std::string _currentLine;
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
	std::optional<MeshData> MeshData::create(const SharedPtr<Blob>& blob)
	{
		ObjParser parser;
		for (Reader reader{ *blob };;)
		{
			auto line = reader.readLine();
			if (line.empty())
				break;
			if (!parser.parseLine(line))
				return std::nullopt;
		}
		return parser.finish();
	}
}
