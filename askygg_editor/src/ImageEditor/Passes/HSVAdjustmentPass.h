#pragma once

#include "ImagePass.h"

struct HSVAdjustmentSettings
{
	float HueShift = 0.0f;
	float SaturationBoost = 0.0f;
	float ValueBoost = 0.0f;
};

class HSVAdjustmentPass : public ImagePass
{
public:
	explicit HSVAdjustmentPass(std::string settingsFilePath);
	uint32_t	GetOutputID() override { return m_HSVOutput->GetID(); }
	std::string GetOutputName() override { return m_OutputName; }
	void		Initialize() override;
	void		Submit(uint32_t textureID) override;
	void		DrawUI() override;
	void		Save() override;
	void		Load() override;
	void		OnResize(const glm::vec2& targetSize) override;

private:
	std::string					   m_OutputName = "HSV Output";
	HSVAdjustmentSettings		   m_Settings;
	askygg::Ref<askygg::Texture2D> m_HSVOutput;
};
