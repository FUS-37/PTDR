#pragma once

#include "IPlug_include_in_plug_hdr.h"

const int kNumPresets = 1;

enum EParams
{
  kMode = 0,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class IPlugEffect final : public Plugin
{
private:
  int mMode = 0; // Current mode (0-3)
  
  float applyGain(float sample, float dB)
  {
    float gain = powf(10.0f, dB / 20.0f);
    return sample * gain;
  }

public:
  IPlugEffect(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
