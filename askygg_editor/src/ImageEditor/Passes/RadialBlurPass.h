#pragma once

#include "ImagePass.h"

struct RadialBlurSettings
{
	float	  BlurStrength = 0.1f;
	glm::vec3 BlurDirection{ 0.1f, 0.1f, 0.0f };
	int		  BlurSamples = 10;
};

class RadialBlurPass : public ImagePass
{
public:
	explicit RadialBlurPass(std::string settingsFilePath);
	uint32_t	GetOutputID() override { return m_RadialBlurOutput->GetID(); }
	std::string GetOutputName() override { return m_OutputName; }
	void		Initialize() override;
	void		Submit(uint32_t textureID) override;
	void		DrawUI() override;
	void		Save() override;
	void		Load() override;
	void		OnResize(const glm::vec2& targetSize) override;

private:
	std::string					   m_OutputName = "Radial Blur Output";
	RadialBlurSettings			   m_Settings;
	askygg::Ref<askygg::Texture2D> m_RadialBlurOutput;
};
