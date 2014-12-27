#pragma once

#include <string>

#include "Memory.h"

namespace engine {

    class SoundSample;

    class SoundSampleDeleter {
    public:
        // Implement this to destroy and release a SoundSample*
        void operator()(SoundSample *);
    };

    using SoundSamplePtr = std::unique_ptr<SoundSample, SoundSampleDeleter>;

    class Sound {

    public:
        Sound();

        void initialize();

        void cleanup();

        SoundSamplePtr loadSample(const std::string filename);

        void playSampleOnce(SoundSample *sample);

        void playSampleRepeated(SoundSample *sample);

        // Volume interval is (0, 1)
        void setVolume(double volume);

        void setSampleVolume(SoundSample *sample, double volume);
    };
}