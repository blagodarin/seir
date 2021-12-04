// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "backend.hpp"

#include <seir_audio/player.hpp>
#include <seir_base/pointer.hpp>
#include <seir_base/scope.hpp>
#include "common.hpp"

#define WIN32_LEAN_AND_MEAN
#include <seir_base/windows_utils.hpp>
#pragma warning(push)
#pragma warning(disable : 4668) // '_WIN64' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#include <audioclient.h>
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable : 4365) // signed/unsigned mismatch
#pragma warning(disable : 5204) // class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
#include <comdef.h>
#pragma warning(pop)
#include <mmdeviceapi.h>

namespace
{
	template <typename T>
	using ComPtr = _com_ptr_t<_com_IIID<T, &__uuidof(T)>>;
}

namespace seir
{
	void runAudioBackend(AudioBackendCallbacks& callbacks, unsigned)
	{
		const auto error = [&callbacks](const char* function, HRESULT code) {
			callbacks.onBackendError(function, static_cast<int>(code), static_cast<const char*>(windows::errorText(static_cast<DWORD>(code))));
		};
		if (const auto hr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED); FAILED(hr))
			return error("CoInitializeEx", hr);
		SEIR_FINALLY([] { ::CoUninitialize(); });
		ComPtr<IMMDeviceEnumerator> deviceEnumerator;
		if (const auto hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&deviceEnumerator)); !deviceEnumerator)
			return error("CoCreateInstance", hr);
		ComPtr<IMMDevice> device;
		if (const auto hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device); !device)
			return hr == E_NOTFOUND ? callbacks.onBackendError(AudioError::NoDevice) : error("IMMDeviceEnumerator::GetDefaultAudioEndpoint", hr);
		ComPtr<IAudioClient> audioClient;
		if (const auto hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&audioClient)); !audioClient)
			return error("IMMDevice::Activate", hr);
		REFERENCE_TIME period = 0;
		if (const auto hr = audioClient->GetDevicePeriod(nullptr, &period); FAILED(hr))
			return error("IAudioClient::GetDevicePeriod", hr);
		CPtr<WAVEFORMATEX, ::CoTaskMemFree> format;
		if (const auto hr = audioClient->GetMixFormat(format.out()); !format)
			return error("IAudioClient::GetMixFormat", hr);
		if (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
		{
			const auto extensible = reinterpret_cast<WAVEFORMATEXTENSIBLE*>(format.get());
			if (!::IsEqualGUID(extensible->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) || extensible->Format.wBitsPerSample != 32)
			{
				extensible->Format.wBitsPerSample = 32;
				extensible->Format.nBlockAlign = static_cast<WORD>((format->wBitsPerSample / 8) * format->nChannels);
				extensible->Format.nAvgBytesPerSec = format->nBlockAlign * format->nSamplesPerSec;
				extensible->Samples.wValidBitsPerSample = 32;
				extensible->SubFormat = KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
			}
		}
		else if (format->wFormatTag != WAVE_FORMAT_IEEE_FLOAT || format->wBitsPerSample != 32)
		{
			format->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
			format->wBitsPerSample = 32;
			format->nBlockAlign = static_cast<WORD>((format->wBitsPerSample / 8) * format->nChannels);
			format->nAvgBytesPerSec = format->nBlockAlign * format->nSamplesPerSec;
		}
		if (format->nChannels != kAudioChannels)
		{
			format->nChannels = kAudioChannels;
			format->nBlockAlign = static_cast<WORD>((format->wBitsPerSample / 8) * format->nChannels);
			format->nAvgBytesPerSec = format->nBlockAlign * format->nSamplesPerSec;
		}
		if (const auto hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, period, 0, format, nullptr); FAILED(hr))
			return error("IAudioClient::Initialize", hr);
		seir::CPtr<std::remove_pointer_t<HANDLE>, ::CloseHandle> event;
		if (*event.out() = ::CreateEventW(nullptr, FALSE, FALSE, nullptr); !event)
			return error("CreateEventW", static_cast<HRESULT>(::GetLastError()));
		if (const auto hr = audioClient->SetEventHandle(event); FAILED(hr))
			return error("IAudioClient::SetEventHandle", hr);
		UINT32 bufferFrames = 0;
		if (const auto hr = audioClient->GetBufferSize(&bufferFrames); FAILED(hr))
			return error("IAudioClient::GetBufferSize", hr);
		ComPtr<IAudioRenderClient> audioRenderClient;
		if (const auto hr = audioClient->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void**>(&audioRenderClient)); !audioRenderClient)
			return error("IAudioClient::GetService", hr);
		callbacks.onBackendAvailable(format->nSamplesPerSec, bufferFrames);
		const UINT32 updateFrames = bufferFrames / kAudioFramesPerBlock * kAudioFramesPerBlock / 2;
		bool audioClientStarted = false;
		SEIR_FINALLY([&] {
			if (audioClientStarted)
				audioClient->Stop();
		});
		while (callbacks.onBackendIdle())
		{
			UINT32 lockedFrames = 0;
			for (;;)
			{
				UINT32 paddingFrames = 0;
				if (const auto hr = audioClient->GetCurrentPadding(&paddingFrames); FAILED(hr))
					return error("IAudioClient::GetCurrentPadding", hr);
				lockedFrames = (bufferFrames - paddingFrames) / kAudioFramesPerBlock * kAudioFramesPerBlock;
				if (lockedFrames >= updateFrames)
					break;
				if (const auto status = ::WaitForSingleObjectEx(event, 2 * paddingFrames * 1000 / format->nSamplesPerSec, FALSE); status != WAIT_OBJECT_0)
					return error("WaitForSingleObjectEx", static_cast<HRESULT>(status == WAIT_TIMEOUT ? ERROR_TIMEOUT : ::GetLastError()));
			}
			BYTE* buffer = nullptr;
			if (const auto hr = audioRenderClient->GetBuffer(lockedFrames, &buffer); FAILED(hr))
				return error("IAudioRenderClient::GetBuffer", hr);
			auto writtenFrames = static_cast<UINT32>(callbacks.onBackendRead(reinterpret_cast<float*>(buffer), lockedFrames));
			DWORD releaseFlags = 0;
			if (!writtenFrames)
			{
				writtenFrames = lockedFrames;
				releaseFlags = AUDCLNT_BUFFERFLAGS_SILENT;
			}
			if (const auto hr = audioRenderClient->ReleaseBuffer(writtenFrames, releaseFlags); FAILED(hr))
				return error("IAudioRenderClient::ReleaseBuffer", hr);
			if (!audioClientStarted)
			{
				if (const auto hr = audioClient->Start(); FAILED(hr))
					return error("IAudioRenderClient::Start", hr);
				audioClientStarted = true;
			}
		}
	}
}
