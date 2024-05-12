// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "window.hpp"

#include <seir_graphics/point.hpp>
#include <seir_graphics/size.hpp>
#include <seir_image/image.hpp>
#include <seir_image/utils.hpp>
#include "app.hpp"

namespace
{
	std::unique_ptr<wchar_t[]> toWChar(const std::string& text)
	{
		if (!text.empty())
		{
			if (const auto wtextSize = ::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), nullptr, 0))
			{
				auto wtext = std::make_unique<wchar_t[]>(wtextSize + 1u);
				if (::MultiByteToWideChar(CP_UTF8, 0, text.c_str(), static_cast<int>(text.size()), wtext.get(), wtextSize))
				{
					wtext[static_cast<size_t>(wtextSize)] = L'\0';
					return wtext;
				}
			}
			seir::windows::reportError("MultiByteToWideChar");
		}
		return {};
	}
}

namespace seir
{
	WindowImpl::WindowImpl(AppImpl& app, Window& window, Hwnd&& hwnd) noexcept
		: _app{ app }
		, _window{ window }
		, _hwnd{ std::move(hwnd) }
	{
		app.addWindow(_hwnd, this);
	}

	void WindowImpl::reset() noexcept
	{
		_hwnd.reset();
	}

	std::unique_ptr<WindowImpl> WindowImpl::create(AppImpl& app, Window& window, const std::string& title)
	{
		const auto wtitle = ::toWChar(title);
		Hwnd hwnd{ ::CreateWindowExW(WS_EX_APPWINDOW, AppImpl::kWindowClass, wtitle ? wtitle.get() : L"", WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, app.instance(), &app) };
		if (!hwnd) [[unlikely]]
		{
			windows::reportError("CreateWindowExW");
			return {};
		}
		return std::make_unique<WindowImpl>(app, window, std::move(hwnd));
	}

	Window::Window(App& app, const std::string& title)
		: _impl{ app._impl ? WindowImpl::create(*app._impl, *this, title) : nullptr }
	{
	}

	Window::~Window() noexcept = default;

	void Window::close() noexcept
	{
		::SendMessageW(_impl->_hwnd, WM_CLOSE, 0, 0);
	}

	std::optional<Point> Window::cursor() const noexcept
	{
		POINT point{ .x = 0, .y = 0 };
		if (!::GetCursorPos(&point))
		{
			seir::windows::reportError("GetCursorPos");
			return {};
		}
		if (!::ScreenToClient(_impl->_hwnd, &point))
			return {};
		return std::make_optional<Point>(point.x, point.y);
	}

	WindowDescriptor Window::descriptor() const noexcept
	{
		return { _impl->_app.instance(), reinterpret_cast<intptr_t>(_impl->_hwnd.get()) };
	}

	void Window::setIcon(const Image& image) noexcept
	{
		const ImageInfo info{ image.info().width(), image.info().height(), PixelFormat::Bgra32, ImageAxes::XRightYUp };
		const auto maskSize = (info.width() + 7) / 8 * info.height();
		const auto bufferSize = sizeof(BITMAPINFOHEADER) + info.frameSize() + maskSize;
		Buffer buffer;
		if (!buffer.tryReserve(bufferSize, 0))
			return;
		auto* header = reinterpret_cast<BITMAPINFOHEADER*>(buffer.data());
		header->biSize = sizeof *header;
		header->biWidth = static_cast<LONG>(info.width());
		header->biHeight = static_cast<LONG>(info.height()) * 2;
		header->biPlanes = 1;
		header->biBitCount = 32;
		header->biCompression = BI_RGB;
		header->biSizeImage = info.frameSize();
		header->biXPelsPerMeter = 0;
		header->biYPelsPerMeter = 0;
		header->biClrUsed = 0;
		header->biClrImportant = 0;
		if (!copyImage(image, info, buffer.data() + sizeof *header))
			return;
		std::memset(buffer.data() + sizeof *header + header->biSizeImage, 0xff, maskSize);
		Hicon icon{ ::CreateIconFromResourceEx(reinterpret_cast<BYTE*>(buffer.data()), static_cast<DWORD>(bufferSize), TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR) };
		if (!icon)
			return windows::reportError("CreateIconFromResourceEx");
		_impl->_icon = std::move(icon);
		::SendMessageW(_impl->_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(_impl->_icon.get()));
		::SendMessageW(_impl->_hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(_impl->_icon.get()));
	}

	void Window::setTitle(const std::string& title) noexcept
	{
		const auto wtitle = ::toWChar(title);
		if (!::SetWindowTextW(_impl->_hwnd, wtitle ? wtitle.get() : L""))
			windows::reportError("SetWindowTextW");
	}

	void Window::show() noexcept
	{
		::ShowWindow(_impl->_hwnd, SW_SHOW);
		::UpdateWindow(_impl->_hwnd);
		::SetForegroundWindow(_impl->_hwnd);
		::SetFocus(_impl->_hwnd);
	}

	Size Window::size() const noexcept
	{
		RECT clientRect{};
		if (!::GetClientRect(_impl->_hwnd, &clientRect))
			windows::reportError("GetClientRect");
		return { static_cast<int>(clientRect.right - clientRect.left), static_cast<int>(clientRect.bottom - clientRect.top) };
	}
}
