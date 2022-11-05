// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "format.hpp"

#include <seir_data/writer.hpp>

#include <cassert>
#include <csetjmp>

#include <jpeglib.h>

static_assert(BITS_IN_JSAMPLE == 8);

namespace
{
	class JpegErrorManager
	{
	public:
		JpegErrorManager()
		{
			::jpeg_std_error(&_errorMgr);
			_errorMgr.error_exit = [](jpeg_common_struct* common) noexcept {
				::jpeg_destroy(common);
				std::longjmp(reinterpret_cast<JpegErrorManager*>(common->client_data)->_jmpBuf, 1); // NOLINT(cert-err52-cpp)
			};
			_errorMgr.output_message = [](jpeg_common_struct*) noexcept {};
		}

	protected:
		std::jmp_buf _jmpBuf{};
		jpeg_error_mgr _errorMgr{};
	};

	class JpegCompressor : private JpegErrorManager
	{
	public:
		explicit JpegCompressor(seir::Writer& writer) noexcept
			: _writer{ writer }
		{
			_destinationMgr.init_destination = [](jpeg_compress_struct* compressor) noexcept {
				const auto output = reinterpret_cast<JpegCompressor*>(compressor->client_data);
				compressor->dest->next_output_byte = reinterpret_cast<JOCTET*>(output->_buffer.data());
				compressor->dest->free_in_buffer = output->_buffer.capacity();
			};
			_destinationMgr.empty_output_buffer = [](jpeg_compress_struct* compressor) noexcept -> boolean {
				const auto output = reinterpret_cast<JpegCompressor*>(compressor->client_data);
				if (!output->_writer.write(output->_buffer.data(), output->_buffer.capacity()))
					compressor->err->error_exit(reinterpret_cast<jpeg_common_struct*>(compressor));
				compressor->dest->init_destination(compressor);
				return TRUE;
			};
			_destinationMgr.term_destination = [](jpeg_compress_struct* compressor) noexcept {
				const auto output = reinterpret_cast<JpegCompressor*>(compressor->client_data);
				if (!output->_writer.write(output->_buffer.data(), output->_buffer.capacity() - compressor->dest->free_in_buffer))
					compressor->err->error_exit(reinterpret_cast<jpeg_common_struct*>(compressor));
			};
		}

		bool compress(const seir::ImageInfo& info, const void* data, J_COLOR_SPACE colorSpace, int compressionLevel) noexcept
		{
			if (!_buffer.tryReserve(65'536, 0))
				return false;
			if (setjmp(_jmpBuf)) // NOLINT(cert-err52-cpp)
				return false;
			jpeg_compress_struct compressor{};
			compressor.err = &_errorMgr;
			compressor.client_data = this;
			::jpeg_create_compress(&compressor);
			compressor.dest = &_destinationMgr;
			compressor.image_width = static_cast<JDIMENSION>(info.width());
			compressor.image_height = static_cast<JDIMENSION>(info.height());
			compressor.input_components = static_cast<int>(pixelSize(info.pixelFormat()));
			compressor.in_color_space = colorSpace;
			::jpeg_set_defaults(&compressor);
			compressor.optimize_coding = TRUE;
			compressor.dct_method = JDCT_ISLOW;
			::jpeg_set_quality(&compressor, 100 - compressionLevel, TRUE);
			::jpeg_start_compress(&compressor, TRUE);
			if (info.axes() == seir::ImageAxes::XRightYDown)
				for (auto row = static_cast<const JSAMPLE*>(data); compressor.next_scanline < compressor.image_height; row += info.stride())
					::jpeg_write_scanlines(&compressor, const_cast<JSAMPLE**>(&row), 1);
			else
				for (auto row = static_cast<const JSAMPLE*>(data) + info.frameSize() - info.stride(); compressor.next_scanline < compressor.image_height; row -= info.stride())
					::jpeg_write_scanlines(&compressor, const_cast<JSAMPLE**>(&row), 1);
			::jpeg_finish_compress(&compressor);
			::jpeg_destroy_compress(&compressor);
			return true;
		}

	private:
		jpeg_destination_mgr _destinationMgr{};
		seir::Writer& _writer;
		seir::Buffer _buffer;
	};

	class JpegDecompressor : private JpegErrorManager
	{
	public:
		explicit JpegDecompressor(const seir::Reader& reader) noexcept
		{
			_sourceMgr.next_input_byte = static_cast<const JOCTET*>(reader.peek(0));
			_sourceMgr.bytes_in_buffer = reader.size() - reader.offset();
			_sourceMgr.init_source = [](jpeg_decompress_struct*) {};
			_sourceMgr.fill_input_buffer = [](jpeg_decompress_struct* decompressor) -> boolean {
				decompressor->err->error_exit(reinterpret_cast<jpeg_common_struct*>(decompressor));
				return FALSE;
			};
			_sourceMgr.skip_input_data = [](jpeg_decompress_struct* decompressor, long bytesToSkip) {
				assert(bytesToSkip > 0);
				if (static_cast<unsigned long>(bytesToSkip) > decompressor->src->bytes_in_buffer)
					decompressor->err->error_exit(reinterpret_cast<jpeg_common_struct*>(decompressor));
				decompressor->src->next_input_byte += bytesToSkip;
				decompressor->src->bytes_in_buffer -= static_cast<unsigned long>(bytesToSkip);
			};
			_sourceMgr.resync_to_restart = ::jpeg_resync_to_restart;
			_sourceMgr.term_source = [](jpeg_decompress_struct*) {};
		}

		bool decompress(seir::ImageInfo& info, seir::Buffer& buffer) noexcept
		{
			if (setjmp(_jmpBuf)) // NOLINT(cert-err52-cpp)
				return false;
			jpeg_decompress_struct decompressor{};
			decompressor.err = &_errorMgr;
			decompressor.client_data = this;
			::jpeg_create_decompress(&decompressor);
			decompressor.src = &_sourceMgr;
			::jpeg_read_header(&decompressor, TRUE);
			seir::PixelFormat pixelFormat; // NOLINT(cppcoreguidelines-init-variables)
			if (decompressor.out_color_space == JCS_GRAYSCALE)
				pixelFormat = seir::PixelFormat::Gray8;
			else
			{
				decompressor.out_color_space = JCS_EXT_BGRA;
				pixelFormat = seir::PixelFormat::Bgra32;
			}
			::jpeg_calc_output_dimensions(&decompressor);
			info = { decompressor.output_width, decompressor.output_height, pixelFormat };
			if (!buffer.tryReserve(info.frameSize(), 0))
			{
				::jpeg_destroy_decompress(&decompressor);
				return false;
			}
			decompressor.do_fancy_upsampling = TRUE;
			::jpeg_start_decompress(&decompressor);
			for (auto scanline = buffer.data(); decompressor.output_scanline < decompressor.output_height; scanline += info.stride())
				::jpeg_read_scanlines(&decompressor, reinterpret_cast<JSAMPLE**>(&scanline), 1);
			::jpeg_finish_decompress(&decompressor);
			::jpeg_destroy_decompress(&decompressor);
			return true;
		}

	private:
		jpeg_source_mgr _sourceMgr{};
	};
}

namespace seir
{
	const void* loadJpegImage(Reader& reader, ImageInfo& info, Buffer& buffer) noexcept
	{
		return JpegDecompressor{ reader }.decompress(info, buffer) ? buffer.data() : nullptr;
	}

	bool saveJpegImage(Writer& writer, const ImageInfo& info, const void* data, int compressionLevel) noexcept
	{
		if (info.axes() != ImageAxes::XRightYDown && info.axes() != ImageAxes::XRightYUp)
			return false;
		if (info.width() > JPEG_MAX_DIMENSION || info.height() > JPEG_MAX_DIMENSION)
			return false;
		J_COLOR_SPACE colorSpace = JCS_UNKNOWN;
		switch (info.pixelFormat())
		{
		case PixelFormat::Gray8: colorSpace = JCS_GRAYSCALE; break;
		case PixelFormat::Intensity8:
		case PixelFormat::GrayAlpha16:
			return false;
		case PixelFormat::Rgb24: colorSpace = JCS_EXT_RGB; break;
		case PixelFormat::Bgr24: colorSpace = JCS_EXT_BGR; break;
		case PixelFormat::Rgba32: colorSpace = JCS_EXT_RGBX; break;
		case PixelFormat::Bgra32: colorSpace = JCS_EXT_BGRX; break;
		}
		return JpegCompressor{ writer }.compress(info, data, colorSpace, compressionLevel);
	}
}
