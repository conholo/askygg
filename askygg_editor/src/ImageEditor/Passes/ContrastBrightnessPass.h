#pragma once

#include "ImagePass.h"

struct ContrastBrightnessSettings
{
	float ContrastStrength = 2.0f;
	float Brightness = -1.0f;
};

class ContrastBrightnessPass : public ImagePass
{
public:
	explicit ContrastBrightnessPass(std::string settingsFilePath);
	uint32_t	GetOutputID() override { return m_ContrastBrightnessOutput->GetID(); }
	std::string GetOutputName() override { return m_OutputName; }
	void		Initialize() override;
	void		Submit(uint32_t textureID) override;
	void		DrawUI() override;
	void		Save() override;
	void		Load() override;
	void		OnResize(const glm::vec2& targetSize) override;

private:
	std::string					   m_OutputName = "Contrast/Brightness Output";
	ContrastBrightnessSettings	   m_Settings;
	askygg::Ref<askygg::Texture2D> m_ContrastBrightnessOutput;
};
