#pragma once

#include "ImagePass.h"

struct SobelSettings
{
	float SobelStrength = 0.0f;
	float Threshold = 0.0f;
};

class SobelPass : public ImagePass
{
public:
	explicit SobelPass(std::string settingsFilePath);
	uint32_t	GetOutputID() override { return m_SobelOutput->GetID(); }
	std::string GetOutputName() override { return m_OutputName; }
	void		Initialize() override;
	void		Submit(uint32_t textureID) override;
	void		DrawUI() override;
	void		Save() override;
	void		Load() override;
	void		OnResize(const glm::vec2& targetSize) override;

private:
	std::string					   m_OutputName = "Sobel Output";
	SobelSettings				   m_Settings;
	askygg::Ref<askygg::Texture2D> m_SobelOutput;
};
