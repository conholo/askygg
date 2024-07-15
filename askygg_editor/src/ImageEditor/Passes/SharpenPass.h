#pragma once

#include "ImagePass.h"

struct SharpenSettings
{
	float SharpenStrength = 0.0f;
};

class SharpenPass : public ImagePass
{
public:
	explicit SharpenPass(std::string settingsFilePath);
	uint32_t	GetOutputID() override { return m_SharpenOutput->GetID(); }
	std::string GetOutputName() override { return m_OutputName; }
	void		Initialize() override;
	void		Submit(uint32_t textureID) override;
	void		DrawUI() override;
	void		Save() override;
	void		Load() override;
	void		OnResize(const glm::vec2& targetSize) override;

private:
	std::string					   m_OutputName = "Sharpen Output";
	SharpenSettings				   m_Settings;
	askygg::Ref<askygg::Texture2D> m_SharpenOutput;
};
