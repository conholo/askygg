#pragma once

#include "askygg/renderer/Texture.h"
#include "askygg/renderer/Shader.h"
#include "PassHelper.h"

enum class ImagePassType
{
	Linearize,
	BarrelDistortion,
	MultiPassBloom,
    RadialBloom,
	ChromaticAberration,
	ContrastBrightness,
	HueShift,
	RadialBlur,
	Sharpen,
	Sobel,
	Vignette,
	OutputCompute,
    Invalid
};

enum class BloomType
{
    None = 0,
    Radial = 1,
    MultiPass = 2
};

class LinearizePass;
class BarrelDistortionPass;
class MultiPassBloomPass;
class RadialBloomPass;
class ChromaticAberrationPass;
class ContrastBrightnessPass;
class HSVAdjustmentPass;
class RadialBlurPass;
class SharpenPass;
class SobelPass;
class VignettePass;
class OutputComputePass;

class ImagePass
{
public:
	explicit ImagePass(std::string settingsFilePath);
	virtual ~ImagePass() = default;

	virtual uint32_t	GetOutputID() = 0;
	virtual std::string GetOutputName() = 0;
	virtual void		Initialize() = 0;
	virtual void		Submit(uint32_t textureID) = 0;
	virtual void		DrawUI() = 0;
	virtual void		Save() = 0;
	virtual void		Load() = 0;
	virtual void		OnResize(const glm::vec2& targetSize);
    virtual bool        IsActive() { return true; }

	static std::string ImagePassTypeToString(ImagePassType type);
    static ImagePassType ImagePassTypeFromBloomType(BloomType bloomType);
    static ImagePassType ImagePassTypeFromString(const std::string& inString);
    static std::vector<ImagePassType> GetAllBasicImagePassTypes();

protected:
	uint32_t					m_WorkGroupSize = 4;
	askygg::Ref<askygg::Shader> m_Shader;
	std::string					m_SettingsFilePath;
	glm::vec2					m_OutputSize{};
};
