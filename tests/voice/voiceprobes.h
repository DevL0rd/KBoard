#pragma once

#include "speechsegmenter.h"

struct SpeechSegmenterProbe
{
    static bool inSpeech(const SpeechSegmenter &segmenter) { return segmenter.m_detector.inSpeech(); }
};
