// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_u8main/u8main.hpp>

#include <memory>

#define NOGDI
#define NOUSER
#include <seir_base/windows_utils.hpp>

namespace
{
	int callMain()
	{
		std::unique_ptr<char[]> buffer;
		std::unique_ptr<char*[]> argv;
		int argc = 0;
		{
			const seir::windows::LocalPtr<LPWSTR> argvW{ ::CommandLineToArgvW(::GetCommandLineW(), &argc) };
			const auto sizes = std::make_unique<int[]>(static_cast<size_t>(argc));
			size_t bufferSize = 0;
			for (size_t i = 0; i < static_cast<size_t>(argc); ++i)
			{
				const auto size = ::WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, nullptr, 0, nullptr, nullptr);
				sizes[i] = size;
				bufferSize += size;
			}
			buffer = std::make_unique<char[]>(bufferSize);
			argv = std::make_unique<char*[]>(static_cast<size_t>(argc));
			size_t bufferOffset = 0;
			for (size_t i = 0; i < static_cast<size_t>(argc); ++i)
			{
				const auto data = buffer.get() + bufferOffset;
				const auto size = sizes[i];
				::WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, data, size, nullptr, nullptr);
				argv[i] = data;
				bufferOffset += size;
			}
		}
		return u8main(argc, argv.get());
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
