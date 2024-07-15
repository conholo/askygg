#pragma once

#include "ImageEditor/ImagePass.h"

struct BarrelDistortionPassSettings
{
	glm::vec3 DistortionStrength{ 0.0f, 0.0f, 0.0f };
};

class BarrelDistortionPass : public ImagePass
{
public:
	explicit BarrelDistortionPass(std::string settingsFilePath);
	uint32_t	GetOutputID() override { return m_BarrelDistortionOutput->GetID(); }
	std::string GetOutputName() override { return m_OutputName; }
	void		Initialize() override;
	void		Submit(uint32_t textureID) override;
	void		DrawUI() override;
	void		Save() override;
	void		Load() override;
	void		OnResize(const glm::vec2& targetSize) override;
    bool        IsActive() override { return glm::length(m_Settings.DistortionStrength) > 0.0f; }

private:
	std::string					   m_OutputName = "Barrel Distortion Output";
	BarrelDistortionPassSettings   m_Settings;
	askygg::Ref<askygg::Texture2D> m_BarrelDistortionOutput;
};
