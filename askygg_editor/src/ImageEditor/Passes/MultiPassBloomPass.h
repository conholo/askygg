#pragma once

#include "ImageEditor/ImagePass.h"

struct MultiPassBloomSettings
{
	bool  DisplayBloomDebug = false;
	float BloomThreshold = 0.4f;
	float BloomKnee = 0.220f;
    float Radius = 1.0f;
    float UpsampleTightenFactor = 16.0f;
};

class MultiPassBloomPass : public ImagePass
{
public:
	explicit MultiPassBloomPass(std::string settingsFilePath);

	uint32_t	   GetOutputID() override { return m_BloomComputeTextures[2]->GetID(); }
	std::string	   GetOutputName() override { return m_OutputName; }

	void Initialize() override;
	void Submit(uint32_t textureID) override;
	void DrawUI() override;
	void Save() override;
	void Load() override;
	void OnResize(const glm::vec2& targetSize) override;

private:
	std::string									m_OutputName = "Multi-Pass Bloom Output";
	std::vector<askygg::Ref<askygg::Texture2D>> m_BloomComputeTextures{};
	MultiPassBloomSettings								m_Settings;
};
