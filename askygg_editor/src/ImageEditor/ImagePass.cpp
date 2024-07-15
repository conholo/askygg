#include "ImagePass.h"
#include "askygg/core/Application.h"

ImagePass::ImagePass(std::string settingsFilePath)
	: m_SettingsFilePath(std::move(settingsFilePath))
{
	const auto& Window = askygg::Application::GetWindow();
	m_OutputSize = { Window.GetWidth(), Window.GetHeight() };
}

void ImagePass::OnResize(const glm::vec2& targetSize)
{
	m_OutputSize = targetSize;
}

std::string ImagePass::ImagePassTypeToString(ImagePassType type)
{
	switch (type)
	{
		case ImagePassType::BarrelDistortion:
			return "BarrelDistortion";
		case ImagePassType::MultiPassBloom:
			return "MultiPassBloom";
		case ImagePassType::RadialBloom:
			return "RadialBloom";
		case ImagePassType::ChromaticAberration:
			return "ChromaticAberration";
		case ImagePassType::ContrastBrightness:
			return "ContrastBrightness";
		case ImagePassType::HueShift:
			return "HueShift";
		case ImagePassType::RadialBlur:
			return "RadialBlur";
		case ImagePassType::Sharpen:
			return "Sharpen";
		case ImagePassType::Sobel:
			return "Sobel";
		case ImagePassType::Vignette:
			return "Vignette";
		case ImagePassType::Linearize:
			return "Linearize";
		case ImagePassType::OutputCompute:
			return "OutputCompute";
		default:
			return "Invalid type";
	}
}

ImagePassType ImagePass::ImagePassTypeFromString(const std::string& inString)
{
    if(std::equal(inString.begin(), inString.end(), "BarrelDistortion"))
        return ImagePassType::BarrelDistortion;
    if(std::equal(inString.begin(), inString.end(), "MultiPassBloom"))
        return ImagePassType::MultiPassBloom;
    if(std::equal(inString.begin(), inString.end(), "RadialBloom"))
        return ImagePassType::RadialBloom;
    if(std::equal(inString.begin(), inString.end(), "ChromaticAberration"))
        return ImagePassType::ChromaticAberration;
    if(std::equal(inString.begin(), inString.end(), "ContrastBrightness"))
        return ImagePassType::ContrastBrightness;
    if(std::equal(inString.begin(), inString.end(), "HueShift"))
        return ImagePassType::HueShift;
    if(std::equal(inString.begin(), inString.end(), "RadialBlur"))
        return ImagePassType::RadialBlur;
    if(std::equal(inString.begin(), inString.end(), "Sharpen"))
        return ImagePassType::Sharpen;
    if(std::equal(inString.begin(), inString.end(), "Sobel"))
        return ImagePassType::Sobel;
    if(std::equal(inString.begin(), inString.end(), "Vignette"))
        return ImagePassType::Vignette;
    if(std::equal(inString.begin(), inString.end(), "OutputCompute"))
        return ImagePassType::OutputCompute;

    YGG_ASSERT(false, "Unable to convert string to ImagePassType.");
}


std::vector<ImagePassType> ImagePass::GetAllBasicImagePassTypes()
{
    std::vector<ImagePassType> availablePasses =
    {
        ImagePassType::BarrelDistortion,
        ImagePassType::ChromaticAberration,
        ImagePassType::ContrastBrightness,
        ImagePassType::HueShift,
        ImagePassType::RadialBlur,
        ImagePassType::Sharpen,
        ImagePassType::Sobel,
        ImagePassType::Vignette,
    };
    return availablePasses;
}

ImagePassType ImagePass::ImagePassTypeFromBloomType(BloomType bloomType)
{
    switch(bloomType)
    {
        case BloomType::Radial:             return ImagePassType::RadialBloom;
        case BloomType::MultiPass:          return ImagePassType::MultiPassBloom;
        default:                            return ImagePassType::Invalid;
    }
}
