#pragma once

#include "ImagePass.h"

struct ChromaticAberrationPassSettings
{
	float Strength = 1.0f;
};

class ChromaticAberrationPass : public ImagePass
{
public:
	explicit ChromaticAberrationPass(std::string settingsFilePath);
	uint32_t	GetOutputID() override { return m_ChromaticAberrationOutput->GetID(); }
	std::string GetOutputName() override { return m_OutputName; }

	void Initialize() override;
	void Submit(uint32_t textureID) override;
	void DrawUI() override;
	void Save() override;
	void Load() override;
	void OnResize(const glm::vec2& targetSize) override;

private:
	std::string						m_OutputName = "Chromatic Aberration Output";
	ChromaticAberrationPassSettings m_Settings;
	askygg::Ref<askygg::Texture2D>	m_ChromaticAberrationOutput;
};
