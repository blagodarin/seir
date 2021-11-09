// This file is part of Seir.
// Copyright (C) Sergei Blagodarin.
// SPDX-License-Identifier: Apache-2.0

#include "backend.hpp"

#include <seir_audio/player.hpp>
#include <seir_base/buffer.hpp>
#include <seir_base/pointer.hpp>
#include <seir_base/scope.hpp>

#include <cstring>

#include <alsa/asoundlib.h>

namespace
{
	std::string functionName(const char* signature)
	{
		while (*signature == ':')
			++signature;
		auto end = signature;
		while (*end != '(')
			++end;
		return { signature, static_cast<size_t>(end - signature) };
	}
}

namespace seir
{
	void runAudioBackend(AudioBackendCallbacks& callbacks, unsigned samplingRate)
	{
#define CHECK_ALSA(call) \
	if (const auto status = call; status < 0) \
	return callbacks.onBackendError(::functionName(#call).c_str(), status, ::snd_strerror(status))

		snd_pcm_uframes_t periodFrames = 0;
		snd_pcm_uframes_t bufferFrames = 0;
		CPtr<snd_pcm_t, ::snd_pcm_close> pcm;
		if (const auto status = ::snd_pcm_open(pcm.out(), "default", SND_PCM_STREAM_PLAYBACK, 0); status < 0)
			return status == -ENOENT ? callbacks.onBackendError(AudioError::NoDevice) : callbacks.onBackendError("snd_pcm_open", status, ::snd_strerror(status));
		{
			CPtr<snd_pcm_hw_params_t, ::snd_pcm_hw_params_free> hw;
			CHECK_ALSA(::snd_pcm_hw_params_malloc(hw.out()));
			CHECK_ALSA(::snd_pcm_hw_params_any(pcm, hw));
			CHECK_ALSA(::snd_pcm_hw_params_set_access(pcm, hw, SND_PCM_ACCESS_RW_INTERLEAVED));
			CHECK_ALSA(::snd_pcm_hw_params_set_format(pcm, hw, SND_PCM_FORMAT_FLOAT));
			CHECK_ALSA(::snd_pcm_hw_params_set_channels(pcm, hw, kAudioBackendChannels));
			CHECK_ALSA(::snd_pcm_hw_params_set_rate(pcm, hw, samplingRate, 0));
			unsigned periods = 2;
			CHECK_ALSA(::snd_pcm_hw_params_set_periods_near(pcm, hw, &periods, nullptr));
			snd_pcm_uframes_t minPeriod = 0;
			int dir = 0;
			CHECK_ALSA(::snd_pcm_hw_params_get_period_size_min(hw, &minPeriod, &dir));
			periodFrames = (minPeriod + kAudioBackendFrameAlignment - 1) / kAudioBackendFrameAlignment * kAudioBackendFrameAlignment;
			CHECK_ALSA(::snd_pcm_hw_params_set_period_size(pcm, hw, periodFrames, periodFrames == minPeriod ? dir : 0));
			CHECK_ALSA(::snd_pcm_hw_params(pcm, hw));
			CHECK_ALSA(::snd_pcm_hw_params_get_period_size(hw, &periodFrames, nullptr));
			CHECK_ALSA(::snd_pcm_hw_params_get_buffer_size(hw, &bufferFrames));
		}
		{
			CPtr<snd_pcm_sw_params_t, ::snd_pcm_sw_params_free> sw;
			CHECK_ALSA(::snd_pcm_sw_params_malloc(sw.out()));
			CHECK_ALSA(::snd_pcm_sw_params_current(pcm, sw));
			CHECK_ALSA(::snd_pcm_sw_params_set_avail_min(pcm, sw, periodFrames));
			CHECK_ALSA(::snd_pcm_sw_params_set_start_threshold(pcm, sw, 1));
			CHECK_ALSA(::snd_pcm_sw_params_set_stop_threshold(pcm, sw, bufferFrames));
			CHECK_ALSA(::snd_pcm_sw_params(pcm, sw));
		}
		seir::Buffer<float, seir::AlignedAllocator<seir::kAudioAlignment>> period{ periodFrames * kAudioBackendChannels };
		callbacks.onBackendAvailable(periodFrames);
		SEIR_FINALLY([&] { ::snd_pcm_drain(pcm); });
		while (callbacks.onBackendIdle())
		{
			auto data = period.data();
			const auto writtenFrames = callbacks.onBackendRead(data, periodFrames);
			std::memset(data + writtenFrames * kAudioBackendChannels, 0, (periodFrames - writtenFrames) * kAudioBackendFrameBytes);
			for (auto framesLeft = periodFrames; framesLeft > 0;)
			{
				const auto result = ::snd_pcm_writei(pcm, data, framesLeft);
				if (result < 0)
				{
					if (result != -EAGAIN)
						CHECK_ALSA(::snd_pcm_recover(pcm, static_cast<int>(result), 1));
					continue;
				}
				if (result == 0)
				{
					::snd_pcm_wait(pcm, static_cast<int>((bufferFrames * 1000 + samplingRate - 1) / samplingRate));
					continue;
				}
				data += static_cast<snd_pcm_uframes_t>(result) * kAudioBackendChannels;
				framesLeft -= static_cast<snd_pcm_uframes_t>(result);
			}
		}
	}
}
