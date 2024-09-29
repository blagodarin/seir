// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include <seir_io/paths.hpp>

#include <seir_base/int_utils.hpp>
#include <seir_base/pointer.hpp>
#include "utils.hpp"

#include <shlobj.h>
#include <versionhelpers.h>

namespace
{
	std::optional<std::string> makeKnownFolderPath(const KNOWNFOLDERID& id, std::string_view relativePath)
	{
		seir::CPtr<wchar_t, ::CoTaskMemFree> prefix;
		if (const auto hr = ::SHGetKnownFolderPath(id, KF_FLAG_CREATE, nullptr, prefix.out()); FAILED(hr))
			seir::windows::reportError("SHGetKnownFolderPath", seir::toUnsigned(hr));
		else if (const seir::windows::WString relativePathW{ relativePath })
		{
			const auto prefixLength = ::wcslen(prefix.get());
			std::wstring fullPath;
			fullPath.reserve(prefixLength + 1 + relativePathW.size());
			fullPath.assign(prefix.get(), prefixLength);
			const std::wstring_view suffix{ relativePathW.c_str(), relativePathW.size() };
			size_t offset = 0;
			for (;;)
			{
				auto separator = suffix.find_first_of(L"\\/", offset);
				if (separator == std::wstring::npos)
					break;
				if (const auto partLength = separator - offset; partLength > 0)
				{
					fullPath += L'\\';
					fullPath.append(suffix.data() + offset, partLength);
					if (!::CreateDirectoryW(fullPath.c_str(), nullptr))
						if (const auto error = ::GetLastError(); error != ERROR_ALREADY_EXISTS)
						{
							seir::windows::reportError("CreateDirectoryW", error);
							return {};
						}
				}
				offset = separator + 1;
			}
			fullPath += L'\\';
			fullPath.append(suffix.data() + offset, suffix.size() - offset);
			if (const seir::windows::U8String result{ fullPath })
				return result.toString();
		}
		return {};
	}
}

namespace seir
{
	std::optional<std::string> makeScreenshotPath(std::string_view relativePath)
	{
		return ::makeKnownFolderPath(::IsWindows8OrGreater() ? FOLDERID_Screenshots : FOLDERID_Pictures, relativePath);
	}

	std::optional<std::string> makeUserDataPath(std::string_view relativePath)
	{
		return ::makeKnownFolderPath(FOLDERID_RoamingAppData, relativePath);
	}

	std::optional<std::string> makeUserStatePath(std::string_view relativePath)
	{
		return ::makeKnownFolderPath(FOLDERID_LocalAppData, relativePath);
	}
}
