#pragma once

#include "keysound.h"
#include "soundmixer.h"

#include <vector>

struct SoundMixerProbe
{
    static int activeVoices(const SoundMixer &mixer)
    {
        int active = 0;
        for (const SoundMixer::Voice &voice : mixer.m_voices) {
            if (voice.data) {
                ++active;
            }
        }
        return active;
    }
};

struct KeySoundProbe
{
    static SoundMixer &mixer(KeySound &sound) { return sound.m_mixer; }

    static std::vector<float> render(KeySound &sound, int frames, int channels = 1)
    {
        std::vector<float> buffer(static_cast<size_t>(frames) * static_cast<size_t>(channels));
        sound.m_mixer.render(buffer, channels);
        return buffer;
    }
};
