// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_main/u8main.hpp>

#include <seir_base/buffer.hpp>

#include <windows.h>

namespace
{
	int callMain()
	{
		seir::Buffer<char> buffer;
		seir::Buffer<char*> argv;
		int argc = 0;
		{
			const seir::CPtr<LPWSTR, ::LocalFree> argvW{ ::CommandLineToArgvW(::GetCommandLineW(), &argc) };
			seir::Buffer<int> sizes{ static_cast<size_t>(argc) };
			size_t bufferSize = 0;
			for (int i = 0; i < argc; ++i)
			{
				const auto size = ::WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
				sizes.data()[i] = size;
				bufferSize += size;
			}
			buffer.reserve(bufferSize);
			argv.reserve(static_cast<size_t>(argc));
			size_t bufferOffset = 0;
			for (int i = 0; i < argc; ++i)
			{
				const auto data = buffer.data() + bufferOffset;
				const auto size = sizes.data()[i];
				::WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, data, size, nullptr, nullptr);
				argv.data()[i] = data;
				bufferOffset += size;
			}
		}
		return u8main(argc, argv.data());
	}
}

int main(int, char**)
{
	return ::callMain();
}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return ::callMain();
}
