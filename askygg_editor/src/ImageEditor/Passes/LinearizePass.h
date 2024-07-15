#pragma once

#include "ImagePass.h"

class LinearizePass : public ImagePass
{
public:
	explicit LinearizePass(std::string settingsFilePath);
	uint32_t	GetOutputID() override { return m_Output->GetID(); }
	std::string GetOutputName() override { return m_OutputName; }
	void		Initialize() override;
	void		Submit(uint32_t textureID) override;
	void		DrawUI() override {}
	void		Save() override {}
	void		Load() override {}
	void		OnResize(const glm::vec2& targetSize) override;

private:
	std::string					   m_OutputName = "Linearize Output";
	askygg::Ref<askygg::Texture2D> m_Output;
};
