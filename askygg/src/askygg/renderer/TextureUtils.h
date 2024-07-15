#pragma once
#include <cstdint>
typedef unsigned int GLenum;

namespace askygg::ImageUtils
{
	enum class Usage
	{
		None = 0,
		Texture,
		Attachment,
		Storage
	};
	enum class WrapMode
	{
		None = 0,
		Repeat,
		ClampToEdge,
		MirroredRepeat
	};
	enum class FilterMode
	{
		None = 0,
		Linear,
		LinearMipLinear,
		LinearMipNearest,
		Nearest,
		NearestMipNearest,
		NearestMipLinear
	};
	enum class ImageInternalFormat
	{
		None = 0,
		FromImage,
		RGBA,
		RGB,
		RG,
		Red,
		DepthStencil,
		Depth,
		R8,
		R16,
		RG8,
		RG16,
		RGB4,
		RGB5,
		RGB8,
		RGB10,
		RGB12,
		RGBA2,
		RGBA4,
		RGBA8,
		RGBA12,
		RGBA16,
		R16F,
		RG16F,
		RGB16F,
		RGBA16F,
		R32F,
		RG32F,
		RGB32F,
		RGBA32F,
	};
	enum class ImageDataLayout
	{
		None = 0,
		FromImage,
		RGBA,
		RGB,
		RG,
		Red,
		RGBAInt,
		RGBInt,
		RGInt,
		RedInt,
		Stencil,
		Depth,
		DepthStencil
	};
	enum class ImageDataType
	{
		None = 0,
		UByte,
		Byte,
		UShort,
		Short,
		UInt,
		Int,
		HalfFloat,
		Float
	};
	enum class TextureAccessLevel
	{
		None = 0,
		ReadOnly,
		WriteOnly,
		ReadWrite
	};
	enum class TextureShaderDataFormat
	{
		None = 0,
		RGBA32F,
		RGBA16F,
		RG32F,
		RG16F,
		R11FG11FB10F,
		R32F,
		R16F,
		RGBA8,
		RGBA
	};

	uint32_t CalculateMipLevelCount(uint32_t width, uint32_t height);

	GLenum ConvertWrapMode(WrapMode wrapMode);
	GLenum ConvertMinMagFilterMode(FilterMode filterMode);
	GLenum ConvertInternalFormatMode(ImageInternalFormat internalFormat);
	GLenum ConvertDataLayoutMode(ImageDataLayout imageDataLayout);
	GLenum ConvertImageDataType(ImageDataType dataType);
	GLenum ConvertTextureAccessLevel(TextureAccessLevel accessLevel);
	GLenum ConvertShaderFormatType(TextureShaderDataFormat shaderDataFormat);
} // namespace askygg::ImageUtils