#include "IPlugEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

IPlugEffect::IPlugEffect(const InstanceInfo& info)
: iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kMode)->InitInt("Mode", 0, 0, 3, "", 1, "");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    
    // Load and set background image
    IBitmap background = pGraphics->LoadBitmap(BGPNG_FN);
    if (background.IsValid())
    {
      pGraphics->AttachControl(new IBitmapControl(0, 0, background));
    }
    else
    {
      // Fallback to gray background if image not found
      pGraphics->AttachPanelBackground(COLOR_GRAY);
    }
    
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT bounds = pGraphics->GetBounds();
    const IRECT innerBounds = bounds.GetPadded(-20.f);
    const IRECT versionBounds = innerBounds.GetFromTRHC(300, 20);
    
    // Mode selector knob
    IRECT knobRect = innerBounds.GetCentredInside(200, 200);
    IVKnobControl* modeKnob = new IVKnobControl(knobRect, kMode, "Mode");
    
    // Set initial value to 0 (Bypass)
    modeKnob->SetValue(0.0);
    
    pGraphics->AttachControl(modeKnob);
    
    // Mode label
    IRECT modeLabelRect = knobRect.GetTranslated(0, 120);
    ITextControl* modeLabel = new ITextControl(modeLabelRect, "Bypass", IText(20, COLOR_BLACK, nullptr, EAlign::Center, EVAlign::Middle));
    pGraphics->AttachControl(modeLabel);
    
    // Update mode label when knob value changes
    modeKnob->SetActionFunction([&, modeLabel](IControl* pControl) {
      int mode = static_cast<int>(pControl->GetValue() * 3.0f + 0.5f);
      std::string modeName;
      switch(mode) {
        case 0: modeName = "Bypass";
          break;
        case 1: modeName = "Soft";
          break;
        case 2: modeName = "Flat";
          break;
        case 3: modeName = "Harsh";
          break;
        default: modeName = "Bypass";
          break;
      }
      modeLabel->SetStr(modeName.c_str());
      modeLabel->SetDirty(true);
      return true;
    });
    
    // Version info
    WDL_String buildInfoStr;
    GetBuildInfoStr(buildInfoStr, __DATE__, __TIME__);
    pGraphics->AttachControl(new ITextControl(versionBounds, buildInfoStr.Get(), DEFAULT_TEXT.WithAlign(EAlign::Far)));
  };
#endif
}

#if IPLUG_DSP
void IPlugEffect::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  // Update mode from parameter
  mMode = GetParam(kMode)->Int();
  
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      float sample = inputs[c][s];
      
      // Apply processing based on current mode
      switch (mMode) {
        case 0: // Bypass mode: keep original signal
          outputs[c][s] = sample;
          break;
        case 1: // Mode1: -2dB then +2dB
          sample = applyGain(sample, -2.0f);
          sample = applyGain(sample, 2.0f);
          outputs[c][s] = sample;
          break;
        case 2: // Mode2: -0.37dB then +0.37dB
          sample = applyGain(sample, -0.37f);
          sample = applyGain(sample, 0.37f);
          outputs[c][s] = sample;
          break;
        case 3: // Mode3: -2dB +2dB -0.37dB +0.37dB
          sample = applyGain(sample, -2.0f);
          sample = applyGain(sample, 2.0f);
          sample = applyGain(sample, -0.37f);
          sample = applyGain(sample, 0.37f);
          outputs[c][s] = sample;
          break;
        default: // Default: bypass
          outputs[c][s] = sample;
          break;
      }
    }
  }
}
#endif
