#pragma once

#include "ImageEditor/ImagePass.h"

struct RadialBloomSettings
{
    float LuminanceThreshold = 0.4f;
    float BloomAmplitude = 1.0f;
    int   BloomRadiusPixels = 7;
    float SigmaScaleFactor = 3.0;
    float BlurColorWeight = 0.5;
};

class RadialBloomPass : public ImagePass
{
public:
    explicit RadialBloomPass(std::string settingsFilePath);

    uint32_t	   GetOutputID() override { return m_Output->GetID(); }
    std::string	   GetOutputName() override { return m_OutputName; }

    void Initialize() override;
    void Submit(uint32_t textureID) override;
    void DrawUI() override;
    void Save() override;
    void Load() override;
    void OnResize(const glm::vec2& targetSize) override;

private:
    std::string									m_OutputName = "Radial Bloom Output";
    askygg::Ref<askygg::Texture2D>              m_Output;
    askygg::Ref<askygg::Texture2D>              m_Intermediate;
    askygg::Ref<askygg::Texture2D>              m_BloomEmitterExtractionOutput;
    RadialBloomSettings						    m_Settings;
};
