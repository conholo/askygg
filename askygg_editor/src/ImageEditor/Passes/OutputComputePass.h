#pragma once

#include "ImagePass.h"

enum class TonemappingType
{
	None = 0,
	ACES = 1,
	Reinhard = 2,
	ReinhardExtended = 3,
};

struct OutputComputePassSettings
{
	BloomType		ActiveBloomType = BloomType::Radial;
	float			BloomIntensity = 6.0f;
	float			BloomDirtIntensity = 0.1f;
	float			BloomUpsampleRadius = 1.0f;
	float			BloomUpsampleTightenFactor = 16.0f;

    float			Exposure = 1.0f;
	TonemappingType Tonemap = TonemappingType::ACES;
	float			WhitePoint = 1.0f;

    float           NoiseAlpha = 1.0f;
    float           NoiseBeta = 1.0f;
    float           NoiseGamma = 0.0f;

    float           NoiseFrequency = 1.0f;
    float           NoiseAmplitude = 1.0f;
};

class OutputComputePass : public ImagePass
{
public:
	explicit OutputComputePass(std::string settingsFilePath);

	uint32_t GetOutputID() override { return m_ByteOutput->GetID(); }

	std::string GetOutputName() override { return m_OutputName; }
    void SetBloomType(BloomType inType) { m_Settings.ActiveBloomType = inType; }

	void Initialize() override;
	void Submit(uint32_t textureID) override;
	void DrawUI() override;

	void Save() override;
	void Load() override;

	void OnResize(const glm::vec2& targetSize) override;

private:
	std::string					   m_OutputName = "Final Output";
	OutputComputePassSettings	   m_Settings;
	askygg::Ref<askygg::Texture2D> m_ByteOutput;

    askygg::Ref<askygg::Texture2D> m_BloomDirtTexture;
	askygg::Ref<askygg::Texture2D> m_SensorNoisePatchTexture;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
};
