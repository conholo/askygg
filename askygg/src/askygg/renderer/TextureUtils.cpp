
#include "askygg/renderer/TextureUtils.h"
#include <glm/glm.hpp>
#include <glad/glad.h>

namespace askygg::ImageUtils
{
	uint32_t CalculateMipLevelCount(uint32_t width, uint32_t height)
	{
		return (uint32_t)std::floor(std::log2(glm::min(width, height))) + 1;
	}

	GLenum ConvertWrapMode(WrapMode wrapMode)
	{
		switch (wrapMode)
		{
			case WrapMode::Repeat:
				return GL_REPEAT;
			case WrapMode::ClampToEdge:
				return GL_CLAMP_TO_EDGE;
			case WrapMode::MirroredRepeat:
				return GL_MIRRORED_REPEAT;
			case WrapMode::None:
				break;
		}

		return 0;
	}

	GLenum ConvertMinMagFilterMode(FilterMode filterMode)
	{
		switch (filterMode)
		{
			case FilterMode::None:
				return 0;
			case FilterMode::Linear:
				return GL_LINEAR;
			case FilterMode::LinearMipLinear:
				return GL_LINEAR_MIPMAP_LINEAR;
			case FilterMode::LinearMipNearest:
				return GL_LINEAR_MIPMAP_NEAREST;
			case FilterMode::Nearest:
				return GL_NEAREST;
			case FilterMode::NearestMipNearest:
				return GL_NEAREST_MIPMAP_NEAREST;
			case FilterMode::NearestMipLinear:
				return GL_NEAREST_MIPMAP_LINEAR;
		}

		return 0;
	}

	GLenum ConvertInternalFormatMode(ImageInternalFormat internalFormat)
	{
		switch (internalFormat)
		{
			case ImageInternalFormat::None:
				return 0;
			case ImageInternalFormat::RGBA:
				return GL_RGBA;
			case ImageInternalFormat::RGB:
				return GL_RGB;
			case ImageInternalFormat::RG:
				return GL_RG;
			case ImageInternalFormat::Red:
				return GL_RED;
			case ImageInternalFormat::DepthStencil:
				return GL_DEPTH_STENCIL;
			case ImageInternalFormat::Depth:
				return GL_DEPTH_COMPONENT;
			case ImageInternalFormat::R8:
				return GL_R8;
			case ImageInternalFormat::R16:
				return GL_R16;
			case ImageInternalFormat::RG8:
				return GL_RG8;
			case ImageInternalFormat::RG16:
				return GL_RG16;
			case ImageInternalFormat::RGB4:
				return GL_RGB4;
			case ImageInternalFormat::RGB5:
				return GL_RGB5;
			case ImageInternalFormat::RGB8:
				return GL_RGB8;
			case ImageInternalFormat::RGB10:
				return GL_RGB10;
			case ImageInternalFormat::RGB12:
				return GL_RGB12;
			case ImageInternalFormat::RGBA2:
				return GL_RGBA2;
			case ImageInternalFormat::RGBA4:
				return GL_RGBA4;
			case ImageInternalFormat::RGBA8:
				return GL_RGBA8;
			case ImageInternalFormat::RGBA12:
				return GL_RGBA12;
			case ImageInternalFormat::RGBA16:
				return GL_RGBA16;
			case ImageInternalFormat::R16F:
				return GL_R16F;
			case ImageInternalFormat::RG16F:
				return GL_RG16F;
			case ImageInternalFormat::RGB16F:
				return GL_RGB16F;
			case ImageInternalFormat::RGBA16F:
				return GL_RGBA16F;
			case ImageInternalFormat::R32F:
				return GL_R32F;
			case ImageInternalFormat::RG32F:
				return GL_RG32F;
			case ImageInternalFormat::RGB32F:
				return GL_RGB32F;
			case ImageInternalFormat::RGBA32F:
				return GL_RGBA32F;
		}

		return 0;
	}

	GLenum ConvertDataLayoutMode(ImageDataLayout pixelDataFormat)
	{
		switch (pixelDataFormat)
		{
			case ImageDataLayout::None:
				return 0;
			case ImageDataLayout::RGBA:
				return GL_RGBA;
			case ImageDataLayout::RGB:
				return GL_RGB;
			case ImageDataLayout::RG:
				return GL_RG;
			case ImageDataLayout::Red:
				return GL_RED;
			case ImageDataLayout::RGBAInt:
				return GL_RGBA_INTEGER;
			case ImageDataLayout::RGBInt:
				return GL_RGB_INTEGER;
			case ImageDataLayout::RGInt:
				return GL_RG_INTEGER;
			case ImageDataLayout::RedInt:
				return GL_RED_INTEGER;
			case ImageDataLayout::Stencil:
				return GL_STENCIL_INDEX;
			case ImageDataLayout::Depth:
				return GL_DEPTH_COMPONENT;
			case ImageDataLayout::DepthStencil:
				return GL_DEPTH_STENCIL;
		}

		return 0;
	}

	GLenum ConvertImageDataType(ImageDataType dataType)
	{
		switch (dataType)
		{
			case ImageDataType::None:
				return 0;
			case ImageDataType::UByte:
				return GL_UNSIGNED_BYTE;
			case ImageDataType::Byte:
				return GL_BYTE;
			case ImageDataType::UShort:
				return GL_UNSIGNED_SHORT;
			case ImageDataType::Short:
				return GL_SHORT;
			case ImageDataType::UInt:
				return GL_UNSIGNED_INT;
			case ImageDataType::Int:
				return GL_INT;
			case ImageDataType::HalfFloat:
				return GL_HALF_FLOAT;
			case ImageDataType::Float:
				return GL_FLOAT;
		}

		return 0;
	}

	GLenum ConvertTextureAccessLevel(TextureAccessLevel accessLevel)
	{
		switch (accessLevel)
		{
			case TextureAccessLevel::None:
				return 0;
			case TextureAccessLevel::ReadOnly:
				return GL_READ_ONLY;
			case TextureAccessLevel::WriteOnly:
				return GL_WRITE_ONLY;
			case TextureAccessLevel::ReadWrite:
				return GL_READ_WRITE;
		}

		return 0;
	}

	GLenum ConvertShaderFormatType(TextureShaderDataFormat shaderDataFormat)
	{
		switch (shaderDataFormat)
		{
			case TextureShaderDataFormat::None:
				return 0;
			case TextureShaderDataFormat::RGBA32F:
				return GL_RGBA32F;
			case TextureShaderDataFormat::RGBA16F:
				return GL_RGBA16F;
			case TextureShaderDataFormat::RG32F:
				return GL_RG32F;
			case TextureShaderDataFormat::RG16F:
				return GL_RG16F;
			case TextureShaderDataFormat::R11FG11FB10F:
				return GL_R11F_G11F_B10F;
			case TextureShaderDataFormat::R32F:
				return GL_R32F;
			case TextureShaderDataFormat::R16F:
				return GL_R16F;
			case TextureShaderDataFormat::RGBA8:
				return GL_RGBA8;
			case TextureShaderDataFormat::RGBA:
				return GL_RGBA;
		}

		return 0;
	}
} // namespace askygg::ImageUtils