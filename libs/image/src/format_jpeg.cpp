// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "format.hpp"

#include <csetjmp>

#include <jpeglib.h>

static_assert(BITS_IN_JSAMPLE == 8);

namespace
{
	struct JpegErrorHandler
	{
		jpeg_error_mgr _errorMgr;
		std::jmp_buf _jmpBuf;
	};
}

namespace seir
{
	const void* loadJpegImage(Reader& reader, ImageInfo& info, Buffer<std::byte>& buffer)
	{
		const auto size = reader.size();
		if (size > std::numeric_limits<unsigned long>::max())
			return nullptr;
		JpegErrorHandler errorHandler;
		errorHandler._errorMgr.error_exit = [](jpeg_common_struct* cinfo) { std::longjmp(reinterpret_cast<JpegErrorHandler*>(cinfo->err)->_jmpBuf, 1); };
		if (setjmp(errorHandler._jmpBuf))
			return nullptr;
		jpeg_decompress_struct decompressor;
		decompressor.err = ::jpeg_std_error(&errorHandler._errorMgr);
		::jpeg_create_decompress(&decompressor);
		if (setjmp(errorHandler._jmpBuf))
		{
			::jpeg_destroy_decompress(&decompressor);
			return nullptr;
		}
		::jpeg_mem_src(&decompressor, static_cast<const unsigned char*>(reader.peek(0)), static_cast<unsigned long>(size));
		::jpeg_read_header(&decompressor, TRUE);
		decompressor.out_color_space = JCS_EXT_BGRA;
		::jpeg_calc_output_dimensions(&decompressor);
		info = { decompressor.output_width, decompressor.output_height, PixelFormat::Bgra32 };
		if (!buffer.tryReserve(info.frameSize(), 0))
		{
			::jpeg_destroy_decompress(&decompressor);
			return nullptr;
		}
		decompressor.do_fancy_upsampling = TRUE;
		::jpeg_start_decompress(&decompressor);
		for (auto scanline = buffer.data(); decompressor.output_scanline < decompressor.output_height; scanline += info.stride())
			::jpeg_read_scanlines(&decompressor, reinterpret_cast<JSAMPLE**>(&scanline), 1);
		::jpeg_finish_decompress(&decompressor);
		if (setjmp(errorHandler._jmpBuf))
			return nullptr;
		::jpeg_destroy_decompress(&decompressor);
		return buffer.data();
	}
}
