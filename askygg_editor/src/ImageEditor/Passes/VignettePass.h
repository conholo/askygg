#pragma once

#include "ImagePass.h"

struct VignetteSettings
{
	float Radius = 0.75f;
	float Softness = 0.45f;
};

class VignettePass : public ImagePass
{
public:
	explicit VignettePass(std::string settingsFilePath);
	uint32_t	GetOutputID() override { return m_VignetteOutput->GetID(); }
	std::string GetOutputName() override { return m_OutputName; }
	void		Initialize() override;
	void		Submit(uint32_t textureID) override;
	void		DrawUI() override;
	void		Save() override;
	void		Load() override;
	void		OnResize(const glm::vec2& targetSize) override;

private:
	std::string					   m_OutputName = "Vignette Output";
	VignetteSettings			   m_Settings;
	askygg::Ref<askygg::Texture2D> m_VignetteOutput;
};
