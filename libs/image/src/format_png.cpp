// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "format.hpp"

#include <seir_base/int_utils.hpp>
#include <seir_data/compression.hpp>
#include <seir_image/utils.hpp>
#include <seir_io/writer.hpp>

#include <array>

namespace
{
	enum class PngChunkType : uint32_t
	{
		IDAT = seir::makeCC('I', 'D', 'A', 'T'),
		IEND = seir::makeCC('I', 'E', 'N', 'D'),
		IHDR = seir::makeCC('I', 'H', 'D', 'R'),
	};

	enum class PngColorType : uint8_t
	{
		Grayscale = 0,
		Truecolor = 2,
		Indexed = 3,
		GrayscaleAlpha = 4,
		TruecolorAlpha = 6,
	};

	enum class PngCompressionMethod : uint8_t
	{
		Zlib = 0,
	};

	enum class PngFilterMethod : uint8_t
	{
		Standard = 0,
	};

	enum class PngStandardFilterType : uint8_t
	{
		None = 0,
		Sub = 1,
		Up = 2,
		Average = 3,
		Paeth = 4,
	};

	enum class PngInterlaceMethod : uint8_t
	{
		None = 0,
		Adam7 = 1,
	};

#pragma pack(push, 1)

	struct PngHeader
	{
		uint64_t signature;
		struct
		{
			uint32_t length;
			PngChunkType type;
			struct
			{
				uint32_t width;
				uint32_t height;
				uint8_t bitDepth;
				PngColorType colorType;
				PngCompressionMethod compressionMethod;
				PngFilterMethod filterMethod;
				PngInterlaceMethod interlaceMethod;
			} data;
			uint32_t crc;
		} ihdr;
		struct
		{
			uint32_t length;
			PngChunkType type;
		} idat;
	};

	struct PngFooter
	{
		struct
		{
			uint32_t crc;
		} idat;
		struct
		{
			uint32_t length;
			PngChunkType type;
			uint32_t crc;
		} iend;
	};

#pragma pack(pop)

	// CRC-32 as defined by the PNG specification.
	// May be of no general use since different formats and protocols
	// use different CRC-32 polynomials and polynomial representations.
	class Crc32
	{
	public:
		constexpr Crc32() noexcept = default;

		constexpr Crc32& process(const void* data, size_t size) noexcept
		{
			for (size_t i = 0; i < size; ++i)
				_value = _table[static_cast<const uint8_t*>(data)[i] ^ (_value & 0xff)] ^ (_value >> 8); // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
			return *this;
		}

		[[nodiscard]] constexpr uint32_t value() const noexcept
		{
			return ~_value;
		}

	private:
		uint32_t _value = 0xffffffff;

		static constexpr std::array<uint32_t, 256> _table{
			0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
			0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
			0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
			0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
			0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
			0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
			0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
			0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
			0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
			0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
			0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
			0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
			0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
			0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
			0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
			0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
			0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
			0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
			0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
			0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
			0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
			0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
			0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
			0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
			0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
			0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
			0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
			0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
			0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
			0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
			0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
			0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
		};
	};
}

namespace seir
{
	bool savePngImage(Writer& writer, const ImageInfo& info, const void* data, int compressionLevel) noexcept
	{
		if (!info.width() || info.width() > std::numeric_limits<uint32_t>::max())
			return false;

		if (!info.height() || info.height() > std::numeric_limits<uint32_t>::max())
			return false;

		const auto [uncompressedPixelFormat, pngColorType] = [&info]() -> std::pair<PixelFormat, PngColorType> {
			switch (info.pixelFormat())
			{
			case PixelFormat::Gray8:
				return { PixelFormat::Gray8, PngColorType::Grayscale };
			case PixelFormat::Intensity8:
			case PixelFormat::GrayAlpha16:
				return { PixelFormat::GrayAlpha16, PngColorType::GrayscaleAlpha };
			case PixelFormat::Rgb24:
			case PixelFormat::Bgr24:
				return { PixelFormat::Rgb24, PngColorType::Truecolor };
			case PixelFormat::Rgba32:
			case PixelFormat::Bgra32:
				return { PixelFormat::Rgba32, PngColorType::TruecolorAlpha };
			}
			std::unreachable();
		}();

		const auto stride = 1 + info.width() * pixelSize(uncompressedPixelFormat);
		const ImageInfo pngInfo{ info.width(), info.height(), stride, uncompressedPixelFormat, ImageAxes::XRightYDown };

		const auto uncompressedSize = pngInfo.frameSize();
		Buffer uncompressedBuffer{ uncompressedSize };
		if (!copyImage(info, data, pngInfo, uncompressedBuffer.data() + 1))
			return false;

		for (size_t i = 0; i < uncompressedSize; i += stride)
			uncompressedBuffer.data()[i] = std::byte{ toUnderlying(PngStandardFilterType::None) };

		Buffer compressedBuffer;
		size_t compressedSize = 0;
		{
			const auto compressor = Compressor::create(Compression::Zlib);
			if (!compressor->prepare(compressionLevel <= 0
						? CompressionLevel::None
						: (compressionLevel < 50
								  ? CompressionLevel::Minimum
								  : (compressionLevel < 100
											? CompressionLevel::Default
											: CompressionLevel::Maximum))))
				return false;
			compressedBuffer.reserve(compressor->maxCompressedSize(uncompressedSize), 0);
			compressedSize = compressor->compress(compressedBuffer.data(), compressedBuffer.capacity(), uncompressedBuffer.data(), uncompressedSize);
			if (!compressedSize)
				return false;
		}

		const PngHeader header{
			.signature = kPngFileID,
			.ihdr{
				.length = bigEndian(uint32_t{ sizeof header.ihdr.data }),
				.type = PngChunkType::IHDR,
				.data{
					.width = bigEndian(info.width()),
					.height = bigEndian(info.height()),
					.bitDepth = 8,
					.colorType = pngColorType,
					.compressionMethod = PngCompressionMethod::Zlib,
					.filterMethod = PngFilterMethod::Standard,
					.interlaceMethod = PngInterlaceMethod::None,
				},
				.crc = bigEndian(Crc32{}.process(&header.ihdr.type, sizeof header.ihdr.type).process(&header.ihdr.data, sizeof header.ihdr.data).value()) },
			.idat{
				.length = bigEndian(static_cast<uint32_t>(compressedSize)),
				.type = PngChunkType::IDAT,
			},
		};

		const PngFooter footer{
			.idat{
				.crc = bigEndian(Crc32{}.process(&header.idat.type, sizeof header.idat.type).process(compressedBuffer.data(), compressedSize).value()),
			},
			.iend{
				.length = 0,
				.type = PngChunkType::IEND,
				.crc = bigEndian(Crc32{}.process(&footer.iend.type, sizeof footer.iend.type).value()),
			},
		};

		return writer.reserve(sizeof header + compressedSize + sizeof footer)
			&& writer.write(header)
			&& writer.write(compressedBuffer.data(), compressedSize)
			&& writer.write(footer);
	}
}
