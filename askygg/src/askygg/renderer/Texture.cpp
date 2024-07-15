#include "askygg/renderer/Texture.h"
#include "askygg/core/Buffer.h"

#include <stbi/stb_image.h>
#include <stbi/stb_image_write.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <filesystem>

namespace askygg
{
	Texture2D::Texture2D(const Texture2DSpecification& specification)
		: m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		GLenum wrapS = ConvertWrapMode(specification.WrapModeS);
		GLenum wrapT = ConvertWrapMode(specification.WrapModeT);
		GLenum minFilter = ConvertMinMagFilterMode(specification.MinFilterMode);
		GLenum magFilter = ConvertMinMagFilterMode(specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
		GLenum dataFormat = ConvertDataLayoutMode(m_Specification.PixelLayoutFormat);
		GLenum dataType = ConvertImageDataType(m_Specification.DataType);

		uint32_t mips = GetMipLevelCount();
		// glTextureStorage2D(m_ID, mips, ConvertInternalFormatMode(specification.InternalFormat),
		//                    specification.Width, specification.Height);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0,
			dataFormat, dataType, nullptr);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	Texture2D::Texture2D(const Texture2DSpecification& specification, void* data)
		: m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		const GLenum wrapS = ConvertWrapMode(specification.WrapModeS);
		const GLenum wrapT = ConvertWrapMode(specification.WrapModeT);
		const GLenum minFilter = ConvertMinMagFilterMode(specification.MinFilterMode);
		const GLenum magFilter = ConvertMinMagFilterMode(specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		const GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
		const GLenum dataFormat = ConvertDataLayoutMode(m_Specification.PixelLayoutFormat);
		const GLenum dataType = ConvertImageDataType(m_Specification.DataType);

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0,
			dataFormat, dataType, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	Texture2D::Texture2D(const std::string& filePath, const Texture2DSpecification& specification)
		: m_Specification(specification), m_FilePath(filePath)
	{
        if (specification.Name == "Texture2D")
		{
			size_t pos = m_FilePath.find_last_of("/") + 1;
			size_t size = m_FilePath.size() - pos;
			m_Name = m_FilePath.substr(pos, size);
		}
        else
        {
            m_Name = specification.Name;
        }

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, ConvertWrapMode(specification.WrapModeS));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, ConvertWrapMode(specification.WrapModeT));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			ConvertMinMagFilterMode(specification.MinFilterMode));
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			ConvertMinMagFilterMode(specification.MagFilterMode));

		stbi_set_flip_vertically_on_load(1);
		int width, height, channels;

		if (stbi_is_hdr(filePath.c_str()))
		{
			m_ImageData.Data = (byte*)stbi_loadf(filePath.c_str(), &width, &height, &channels, 4);
			m_Specification.InternalFormat = ImageUtils::ImageInternalFormat::RGBA32F;
			m_Specification.PixelLayoutFormat = ImageUtils::ImageDataLayout::RGBA;
			m_Specification.DataType = ImageUtils::ImageDataType::Float;
		}
		else
		{
			m_ImageData.Data = stbi_load(filePath.c_str(), &width, &height, &channels, 4);
			m_Specification.InternalFormat = ImageUtils::ImageInternalFormat::RGBA8;
			m_Specification.PixelLayoutFormat = ImageUtils::ImageDataLayout::RGBA;
			m_Specification.DataType = ImageUtils::ImageDataType::UByte;
		}
		YGG_ASSERT(m_ImageData.Data, "Failed to load image from file: {}!", filePath);
		m_Specification.Width = width;
		m_Specification.Height = height;

		GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
		GLenum dataFormat = ConvertDataLayoutMode(m_Specification.PixelLayoutFormat);
		GLenum dataType = ConvertImageDataType(m_Specification.DataType);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Specification.Width, m_Specification.Height, 0,
			dataFormat, dataType, m_ImageData.Data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	void Texture2D::ReadColorData(const Buffer& buffer, int32_t width, int32_t height, uint32_t id)
	{
		glReadBuffer(id);
		glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, buffer.Data);
	}

	Texture2D::~Texture2D()
	{
		if (m_ID != UINT32_MAX)
			glDeleteTextures(1, &m_ID);
		if (m_ImageData)
		{
			stbi_image_free(m_ImageData.Data);
			m_ImageData.Data = nullptr;
		}

		m_ID = UINT32_MAX;
	}

	void Texture2D::Invalidate()
	{
		if (m_ID != UINT32_MAX)
		{
			glDeleteTextures(1, &m_ID);
			m_ID = UINT32_MAX;
		}

		glCreateTextures(GL_TEXTURE_2D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D, m_ID);

		GLenum wrapS = ConvertWrapMode(m_Specification.WrapModeS);
		GLenum wrapT = ConvertWrapMode(m_Specification.WrapModeT);
		GLenum minFilter = ConvertMinMagFilterMode(m_Specification.MinFilterMode);
		GLenum magFilter = ConvertMinMagFilterMode(m_Specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		uint32_t mips = GetMipLevelCount();
		glTextureStorage2D(m_ID, mips, ConvertInternalFormatMode(m_Specification.InternalFormat),
			m_Specification.Width, m_Specification.Height);
	}

	void Texture2D::ClearImage() const
	{
		glClearTexImage(m_ID, 0, ConvertDataLayoutMode(m_Specification.PixelLayoutFormat),
			ImageUtils::ConvertImageDataType(m_Specification.DataType), nullptr);
	}

	void Texture2D::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	std::pair<uint32_t, uint32_t> Texture2D::GetMipSize(uint32_t mip) const
	{
		uint32_t width = m_Specification.Width;
		uint32_t height = m_Specification.Height;
		while (mip != 0)
		{
			width /= 2;
			height /= 2;
			mip--;
		}

		return { width, height };
	}

	uint32_t Texture2D::GetMipLevelCount() const
	{
		return ImageUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height);
	}

	void Texture2D::SaveToFile(uint32_t TextureID, const std::string& FilePath)
	{
		Texture2D& WriteTexture = TextureLibrary::Get2DFromID(TextureID);
		Buffer	   Data = WriteTexture.GetData();

		stbi_write_hdr(FilePath.c_str(), WriteTexture.GetWidth(), WriteTexture.GetHeight(), 4,
			Data.As<float>());
	}

	void Texture2D::BindTextureIDToSamplerSlot(uint32_t slot, uint32_t id)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, id);
	}

	void Texture2D::BindToSamplerSlot(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void Texture2D::ClearBinding()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void Texture2D::BindToImageSlot(uint32_t unit, uint32_t level,
		ImageUtils::TextureAccessLevel		access,
		ImageUtils::TextureShaderDataFormat shaderDataFormat)
	{
		GLenum glShaderDataFormat = ImageUtils::ConvertShaderFormatType(shaderDataFormat);
		GLenum internalFormat = ImageUtils::ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			YGG_LOG_ERROR("Failure Binding '{}' Texture2D to Image Slot: Shader Data Format and Internal "
						  "format must match!",
				m_Specification.Name);
			return;
		}

		glBindImageTexture(unit, m_ID, level, GL_FALSE, 0, ImageUtils::ConvertTextureAccessLevel(access),
			ImageUtils::ConvertShaderFormatType(shaderDataFormat));
	}

	void Texture2D::SetData(void* data, uint32_t size) const
	{
		uint32_t bytesPerPixel =
			m_Specification.PixelLayoutFormat == ImageUtils::ImageDataLayout::RGBA ? 4 : 3;
		YGG_ASSERT(size == bytesPerPixel * m_Specification.Width * m_Specification.Height,
			"Data size must match entire texture.");
		GLenum pixelLayout = ConvertDataLayoutMode(m_Specification.PixelLayoutFormat);
		GLenum type = ConvertImageDataType(m_Specification.DataType);
		glTextureSubImage2D(m_ID, 0, 0, 0, m_Specification.Width, m_Specification.Height, pixelLayout,
			type, data);
	}

	void Texture2D::SaveFramebufferAttachment(const std::string& filePath, uint32_t id,
		uint32_t bytesPerPixel, uint32_t width, uint32_t height)
	{
		Buffer buffer;
		buffer.Allocate(bytesPerPixel * width * height);

		glBindFramebuffer(GL_FRAMEBUFFER, id);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, buffer.Data);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		std::vector<unsigned char> data8Bit(width * height * 4);
		auto*					   bufferData = buffer.As<float>();

		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				int srcIndex = (height - 1 - y) * width + x;
				int dstIndex = y * width + x;

				glm::vec3 pixel(bufferData[srcIndex * 4 + 0], bufferData[srcIndex * 4 + 1],
					bufferData[srcIndex * 4 + 2]);

				pixel = glm::clamp(pixel, 0.0f, 1.0f);

				// Convert to 8-bit and write to output buffer
				data8Bit[dstIndex * 4 + 0] = static_cast<unsigned char>(pixel.r * 255.0f);
				data8Bit[dstIndex * 4 + 1] = static_cast<unsigned char>(pixel.g * 255.0f);
				data8Bit[dstIndex * 4 + 2] = static_cast<unsigned char>(pixel.b * 255.0f);
				data8Bit[dstIndex * 4 + 3] =
					static_cast<unsigned char>(bufferData[srcIndex * 4 + 3] * 255.0f);
			}
		}

		int result = stbi_write_png(filePath.c_str(), width, height, 4, data8Bit.data(), width * 4);

		if (result == 0)
		{
			YGG_LOG_ERROR("Failed to save image: {}", filePath);
		}
		else
		{
			YGG_LOG_INFO("Image saved to: {}", filePath);
		}

		if (buffer)
		{
			// YGG_LOG_INFO("Freeing {} MB", buffer.Size / static_cast<float>((1024 * 1024)));
			buffer.Release();
		}
	}

	void Texture2D::BindTextureIDToImageSlot(uint32_t id, uint32_t unit, uint32_t level,
		ImageUtils::TextureAccessLevel		access,
		ImageUtils::TextureShaderDataFormat shaderDataFormat)
	{
		glBindImageTexture(unit, id, level, GL_FALSE, 0, ImageUtils::ConvertTextureAccessLevel(access),
			ImageUtils::ConvertShaderFormatType(shaderDataFormat));
	}

	void Texture2D::Save(const std::string& filePath, bool flipVertically) const
	{
		std::vector<unsigned char> buffer(m_Specification.Width * m_Specification.Height * 4);

		glBindTexture(GL_TEXTURE_2D, m_ID);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());

		stbi_flip_vertically_on_write(flipVertically);
		int result = stbi_write_jpg(filePath.c_str(), m_Specification.Width, m_Specification.Height, 4,
			buffer.data(), 0);

		if (result == 0)
		{
			YGG_LOG_ERROR("Failed to save image: {}", filePath);
		}
		else
		{
			YGG_LOG_INFO("Image saved to: {}", filePath);
		}
	}

	Texture2DArray::Texture2DArray(const Texture2DArraySpecification& Specification)
		: m_Specification(Specification), m_Name((Specification.Name))
	{
		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_ID);

		GLenum wrapS = ConvertWrapMode(m_Specification.WrapModeS);
		GLenum wrapT = ConvertWrapMode(m_Specification.WrapModeT);
		GLenum minFilter = ConvertMinMagFilterMode(m_Specification.MinFilterMode);
		GLenum magFilter = ConvertMinMagFilterMode(m_Specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilter);

		const uint32_t mips = GetMipLevelCount();
		glTextureStorage3D(m_ID, mips, ConvertInternalFormatMode(m_Specification.InternalFormat),
			m_Specification.Width, m_Specification.Height, m_Specification.LayerCount);
	}

	Texture2DArray::~Texture2DArray()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture2DArray::Invalidate()
	{
		if (m_ID)
			glDeleteTextures(1, &m_ID);

		glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_ID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_ID);

		GLenum wrapS = ConvertWrapMode(m_Specification.WrapModeS);
		GLenum wrapT = ConvertWrapMode(m_Specification.WrapModeT);
		GLenum minFilter = ConvertMinMagFilterMode(m_Specification.MinFilterMode);
		GLenum magFilter = ConvertMinMagFilterMode(m_Specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilter);

		const uint32_t mips = GetMipLevelCount();
		glTextureStorage3D(m_ID, mips, ConvertInternalFormatMode(m_Specification.InternalFormat),
			m_Specification.Width, m_Specification.Height, m_Specification.LayerCount);
	}

	void Texture2DArray::ResizeLayers(uint32_t width, uint32_t height) {}

	uint32_t Texture2DArray::GetMipLevelCount() const
	{
		return ImageUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height);
	}

	void Texture2DArray::SetDataAtLayer(void* data, size_t size, uint32_t layer)
	{
		const uint32_t bytesPerPixel =
			m_Specification.PixelLayoutFormat == ImageUtils::ImageDataLayout::RGBA ? 4 : 3;
		YGG_ASSERT(size == bytesPerPixel * m_Specification.Width * m_Specification.Height,
			"Data size must match entire texture.");
		GLenum pixelLayout = ConvertDataLayoutMode(m_Specification.PixelLayoutFormat);
		GLenum type = ConvertImageDataType(m_Specification.DataType);
		glTextureSubImage3D(m_ID, 0, 0, 0, 0, m_Specification.Width, m_Specification.Height, layer,
			pixelLayout, type, data);
	}

	void Texture2DArray::SetDataAllLayers(void* data, size_t size)
	{
		for (uint32_t i = 0; i < m_Specification.LayerCount; i++)
			SetDataAtLayer(data, size, i);
	}

	void Texture2DArray::BindTextureIDToSamplerSlot(uint32_t RendererID, uint32_t Slot)
	{
		glActiveTexture(GL_TEXTURE0 + Slot);
		glBindTextureUnit(Slot, RendererID);
	}

	void Texture2DArray::ClearBinding()
	{
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	void Texture2DArray::BindLayerToImageSlot(uint32_t unit, uint32_t level, uint32_t layer,
		ImageUtils::TextureAccessLevel		access,
		ImageUtils::TextureShaderDataFormat shaderDataFormat)
	{
		const GLenum glShaderDataFormat = ConvertShaderFormatType(shaderDataFormat);
		const GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			YGG_LOG_ERROR("Failure Binding '{}' Texture2DArray to Image Slot: Shader Data Format and "
						  "Internal format must match!",
				m_Specification.Name);
			return;
		}

		glBindImageTexture(unit, m_ID, level, GL_TRUE, layer, ConvertTextureAccessLevel(access),
			ConvertShaderFormatType(shaderDataFormat));
	}

	void Texture2DArray::BindAllLayersToImageSlot(
		uint32_t unit, uint32_t level, ImageUtils::TextureAccessLevel access,
		ImageUtils::TextureShaderDataFormat shaderDataFormat)
	{
		const GLenum glShaderDataFormat = ConvertShaderFormatType(shaderDataFormat);
		const GLenum internalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (glShaderDataFormat != internalFormat)
		{
			YGG_LOG_ERROR("Failure Binding '{}' Texture2DArray to Image Slot: Shader Data Format and "
						  "Internal format must match!",
				m_Specification.Name);
			return;
		}

		glBindImageTexture(unit, m_ID, level, GL_FALSE, 0, ConvertTextureAccessLevel(access),
			ConvertShaderFormatType(shaderDataFormat));
	}

	TextureCube::TextureCube(const TextureCubeSpecification& specification,
		const std::vector<std::string>&						 cubeFaceFiles)
		: m_Specification(specification), m_Name(specification.Name)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		int Width, Height, Channels;

		for (uint32_t i = 0; i < cubeFaceFiles.size(); i++)
		{
			if (unsigned char* Data = stbi_load(cubeFaceFiles[i].c_str(), &Width, &Height, &Channels, 0))
			{
				m_Specification.InternalFormat = Channels == 4 ? ImageUtils::ImageInternalFormat::RGBA8
															   : ImageUtils::ImageInternalFormat::RGB8;
				m_Specification.DataLayout =
					Channels == 4 ? ImageUtils::ImageDataLayout::RGBA : ImageUtils::ImageDataLayout::RGB;

				const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);
				const GLenum DataFormat = ConvertDataLayoutMode(m_Specification.DataLayout);
				const GLenum DataType = ConvertImageDataType(m_Specification.DataType);
				m_Specification.Dimension = Height;

				const auto MipCount = GetMipLevelCount();
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, MipCount, InternalFormat, Width, Height, 0,
					DataFormat, DataType, Data);
				stbi_image_free(Data);
			}
			else
			{
				YGG_LOG_ERROR("Failed to load face: {} for Cube Map Texture from file '{}'.", i,
					cubeFaceFiles[i]);
				stbi_image_free(Data);
			}
		}
		const GLenum WrapModeS = ConvertWrapMode(m_Specification.SamplerWrapS);
		const GLenum WrapModeT = ConvertWrapMode(m_Specification.SamplerWrapT);
		const GLenum WrapModeR = ConvertWrapMode(m_Specification.SamplerWrapR);
		const GLenum MinFilter = ConvertMinMagFilterMode(m_Specification.MinFilter);
		const GLenum MagFilter = ConvertMinMagFilterMode(m_Specification.MagFilter);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapModeS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapModeT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapModeR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
	}

	/**
	 * \brief A Cubemap.
	 * \param Specification The Cubemap Specification.
	 */
	TextureCube::TextureCube(const TextureCubeSpecification& Specification)
		: m_Specification(Specification), m_ID(0), m_Name(Specification.Name)
	{
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		const GLenum WrapModeS = ConvertWrapMode(m_Specification.SamplerWrapS);
		const GLenum WrapModeT = ConvertWrapMode(m_Specification.SamplerWrapT);
		const GLenum WrapModeR = ConvertWrapMode(m_Specification.SamplerWrapR);
		const GLenum MinFilter = ConvertMinMagFilterMode(m_Specification.MinFilter);
		const GLenum MagFilter = ConvertMinMagFilterMode(m_Specification.MagFilter);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapModeS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapModeT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapModeR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
		const auto MipCount = GetMipLevelCount();
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, MipCount, InternalFormat, Specification.Dimension,
			Specification.Dimension);
	}

	void TextureCube::Invalidate(const TextureCubeSpecification& Specification)
	{
		YGG_LOG_TRACE("Invalidating Cubemap: ", m_Specification.Name);
		if (Specification.Name != m_Specification.Name)
			YGG_LOG_TRACE("\t New name for Invalidated Cubemap: {}", m_Specification.Name);
		m_Specification = Specification;
		glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_ID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_ID);

		const GLenum WrapModeS = ConvertWrapMode(m_Specification.SamplerWrapS);
		const GLenum WrapModeT = ConvertWrapMode(m_Specification.SamplerWrapT);
		const GLenum WrapModeR = ConvertWrapMode(m_Specification.SamplerWrapR);
		const GLenum MinFilter = ConvertMinMagFilterMode(m_Specification.MinFilter);
		const GLenum MagFilter = ConvertMinMagFilterMode(m_Specification.MagFilter);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapModeS);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapModeT);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapModeR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, MagFilter);
		const auto MipCount = GetMipLevelCount();
		glTexStorage2D(GL_TEXTURE_CUBE_MAP, MipCount, InternalFormat, Specification.Dimension,
			Specification.Dimension);
	}

	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &m_ID);
	}

	void TextureCube::BindToSamplerSlot(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void TextureCube::Clear()
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	}

	void TextureCube::BindToImageSlot(uint32_t Binding, uint32_t MipLevel,
		ImageUtils::TextureAccessLevel		AccessLevel,
		ImageUtils::TextureShaderDataFormat ShaderDataFormat) const
	{
		const GLenum GLShaderDataFormat = ConvertShaderFormatType(ShaderDataFormat);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (GLShaderDataFormat != InternalFormat)
		{
			YGG_LOG_ERROR("Failure Binding '{}' TextureCube to Image Slot: Shader Data Format and Internal "
						  "format must match!",
				m_Specification.Name);
			return;
		}

		glBindImageTexture(Binding, m_ID, MipLevel, GL_TRUE, 0, ConvertTextureAccessLevel(AccessLevel),
			ConvertShaderFormatType(ShaderDataFormat));
	}

	std::pair<glm::uint32_t, glm::uint32_t> TextureCube::GetMipSize(uint32_t Mip) const
	{
		uint32_t Width = m_Specification.Dimension;
		uint32_t Height = m_Specification.Dimension;
		while (Mip != 0)
		{
			Width /= 2;
			Height /= 2;
			Mip--;
		}

		return { Width, Height };
	}

	uint32_t TextureCube::GetMipLevelCount() const
	{
		return ImageUtils::CalculateMipLevelCount(m_Specification.Dimension, m_Specification.Dimension);
	}

	void TextureCube::SetData(uint32_t* Data, size_t Size)
	{
		const uint32_t BytesPerPixel =
			m_Specification.DataLayout == ImageUtils::ImageDataLayout::RGBA ? 4 : 3;
		YGG_ASSERT(Size == BytesPerPixel * m_Specification.Dimension * m_Specification.Dimension,
			"Data size must match entire face of texture.");

		const GLenum PixelLayout = ConvertDataLayoutMode(m_Specification.DataLayout);
		const GLenum DataType = ConvertImageDataType(m_Specification.DataType);

		const std::vector Faces = {
			GL_TEXTURE_CUBE_MAP_POSITIVE_X,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
			GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
			GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		};

		for (const auto Face : Faces)
			glTexSubImage2D(Face, 0, 0, 0, m_Specification.Dimension, m_Specification.Dimension,
				PixelLayout, DataType, Data);
	}

	Texture2DImageView::Texture2DImageView(const Ref<Texture2D>& original, uint32_t baseMip,
		uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
		: m_BaseMip(baseMip)
	{
		glGenTextures(1, &m_ID);
		const GLenum InternalFormat =
			ConvertInternalFormatMode(original->GetSpecification().InternalFormat);
		glTextureView(m_ID, GL_TEXTURE_2D, original->GetID(), InternalFormat, baseMip, mipCount,
			baseLayer, layerCount);
	}

	Texture2DImageView::~Texture2DImageView()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture2DImageView::ChangeToMip(const Ref<Texture2D>& original, uint32_t baseMip,
		uint32_t mipCount, uint32_t baseLayer, uint32_t layerCount)
	{
		if (m_ID)
			glDeleteTextures(1, &m_ID);

		original->BindToSamplerSlot(0);
		m_BaseMip = baseMip;
		glGenTextures(1, &m_ID);
		const GLenum InternalFormat =
			ConvertInternalFormatMode(original->GetSpecification().InternalFormat);
		glTextureView(m_ID, GL_TEXTURE_2D, original->GetID(), InternalFormat, baseMip, mipCount,
			baseLayer, layerCount);
	}

	void Texture2DImageView::Bind() const
	{
		glBindTexture(GL_TEXTURE_2D, m_ID);
	}

	void Texture2DImageView::Unbind()
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	Texture3D::Texture3D(const Texture3DSpecification& Specification)
		: m_Name(Specification.Name)
	{
		glCreateTextures(GL_TEXTURE_3D, 1, &m_ID);
		glBindTexture(GL_TEXTURE_3D, m_ID);

		const GLenum WrapS = ImageUtils::ConvertWrapMode(Specification.WrapModeS);
		const GLenum WrapT = ImageUtils::ConvertWrapMode(Specification.WrapModeT);
		const GLenum WrapR = ImageUtils::ConvertWrapMode(Specification.WrapModeR);
		const GLenum MinFilter = ImageUtils::ConvertMinMagFilterMode(Specification.MinFilterMode);
		const GLenum MagFilter = ImageUtils::ConvertMinMagFilterMode(Specification.MagFilterMode);

		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, WrapS);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, WrapT);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, WrapR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, MinFilter);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, MagFilter);

		// Immutable-format Texture
		// Contents of the image can be modified, but it's storage requirements may not change.
		// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexStorage2D.xhtml

		const uint32_t Mips = GetMipLevelCount();
		glTextureStorage3D(m_ID, Mips, ConvertInternalFormatMode(Specification.InternalFormat),
			Specification.Width, Specification.Height, Specification.Depth);
	}

	Texture3D::~Texture3D()
	{
		glDeleteTextures(1, &m_ID);
	}

	void Texture3D::BindToSamplerSlot(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_ID);
	}

	void Texture3D::Unbind()
	{
		glBindTexture(GL_TEXTURE_3D, 0);
	}

	void Texture3D::BindToImageSlot(uint32_t unit, uint32_t level,
		ImageUtils::TextureAccessLevel		access,
		ImageUtils::TextureShaderDataFormat shaderDataFormat) const
	{
		const GLenum GLShaderDataFormat = ConvertShaderFormatType(shaderDataFormat);
		const GLenum InternalFormat = ConvertInternalFormatMode(m_Specification.InternalFormat);

		if (GLShaderDataFormat != InternalFormat)
		{
			YGG_LOG_ERROR("Failure Binding '{}' Texture3D to Image Slot: Shader Data Format and Internal "
						  "format must match!",
				m_Specification.Name);
			return;
		}

		glBindImageTexture(unit, m_ID, level, GL_TRUE, 0, ConvertTextureAccessLevel(access),
			ConvertShaderFormatType(shaderDataFormat));
	}

	int Texture3D::WriteToFile(const std::string& assetPath) const
	{
		const GLenum  DataFormat = ConvertDataLayoutMode(m_Specification.PixelLayoutFormat);
		const GLenum  DataType = ConvertImageDataType(m_Specification.DataType);
		const GLsizei TextureDataSize =
			m_Specification.Width * m_Specification.Height * m_Specification.Depth * 16;
		void* pixels = malloc(TextureDataSize);
		glGetTextureSubImage(m_ID, 0, 0, 0, 0, m_Specification.Width, m_Specification.Height,
			m_Specification.Width, DataFormat, DataType, TextureDataSize, pixels);
		const int result = stbi_write_png(assetPath.c_str(), m_Specification.Width,
			m_Specification.Height, 4, pixels, 4);
		free(pixels);
		return result;
	}

	std::pair<glm::uint32_t, glm::uint32_t> Texture3D::GetMipSize(uint32_t mip) const
	{
		uint32_t width = m_Specification.Width;
		uint32_t height = m_Specification.Height;
		while (mip != 0)
		{
			width /= 2;
			height /= 2;
			mip--;
		}

		return { width, height };
	}

	uint32_t Texture3D::GetMipLevelCount() const
	{
		return ImageUtils::CalculateMipLevelCount(m_Specification.Width, m_Specification.Height);
	}

	bool TextureLibrary::Has2DFromID(uint32_t id)
	{
		return s_IdToNameLibrary.find(id) != s_IdToNameLibrary.end();
	}

	bool TextureLibrary::Has2D(const std::string& Name)
	{
		return s_NameToTexture2DLibrary.find(Name) != s_NameToTexture2DLibrary.end();
	}

	void TextureLibrary::AddTexture2D(const Ref<Texture2D>& texture)
	{
		if (Has2D(texture->GetName()))
		{
			YGG_LOG_TRACE("Texture2D with name '{}' already contained in Texture Library.",
				texture->GetName());
			return;
		}

		s_NameToTexture2DLibrary[texture->GetName()] = texture;
		s_IdToNameLibrary[texture->GetID()] = texture->GetName();
		s_NameToIDLibrary[texture->GetName()] = texture->GetID();
		YGG_LOG_TRACE("Added Texture2D with name: '{}' to the Texture Library.", texture->GetName());
	}

	Ref<Texture2D> TextureLibrary::LoadTexture2D(const Texture2DSpecification& spec,
		const std::string&													   filePath)
	{
		if (!filePath.empty())
		{
			Ref<Texture2D> texture = CreateRef<Texture2D>(filePath, spec);
			AddTexture2D(texture);
			return texture;
		}

		Ref<Texture2D> texture = CreateRef<Texture2D>(spec);
		AddTexture2D(texture);
		return texture;
	}

	bool TextureLibrary::HasCube(const std::string& Name)
	{
		return s_NameToTextureCubeLibrary.find(Name) != s_NameToTextureCubeLibrary.end();
	}

	void TextureLibrary::AddTextureCube(const Ref<TextureCube>& texture)
	{
		if (HasCube(texture->GetName()))
		{
			YGG_LOG_TRACE("TextureCube with name '{}' already contained in Texture Library.",
				texture->GetName());
			return;
		}

		s_NameToTextureCubeLibrary[texture->GetName()] = texture;
		s_IdToNameLibrary[texture->GetID()] = texture->GetName();
		s_NameToIDLibrary[texture->GetName()] = texture->GetID();
		YGG_LOG_TRACE("Added TextureCube with name: '{}' to the Texture Library.", texture->GetName());
	}

	Ref<TextureCube> TextureLibrary::LoadTextureCube(const TextureCubeSpecification& Spec,
		bool																		 InvalidateIfExists)
	{
		if (HasCube(Spec.Name))
		{
			if (InvalidateIfExists)
			{
				s_NameToTextureCubeLibrary[Spec.Name]->Invalidate(Spec);
				s_NameToIDLibrary[Spec.Name] = s_NameToTextureCubeLibrary[Spec.Name]->GetID();
			}

			return s_NameToTextureCubeLibrary[Spec.Name];
		}
		Ref<TextureCube> texture = CreateRef<TextureCube>(Spec);
		AddTextureCube(texture);
		return texture;
	}

	void TextureLibrary::InvalidateCube(const TextureCubeSpecification& spec)
	{
		YGG_ASSERT(HasCube(spec.Name), "Unable to Invalidate TextureCube with name '{}' - does not exist",
			spec.Name);
		const Ref<TextureCube> TextureCube = s_NameToTextureCubeLibrary[spec.Name];
		TextureCube->Invalidate(spec);
	}

	Texture2D& TextureLibrary::Get2DFromID(uint32_t ID)
	{
		YGG_ASSERT(s_IdToNameLibrary.find(ID) != s_IdToNameLibrary.end(),
			"Unable to find Texture2D with ID: {}", ID);
		const auto TextureName = s_IdToNameLibrary[ID];
		YGG_ASSERT(s_NameToTexture2DLibrary.find(TextureName) != s_NameToTexture2DLibrary.end(),
			"Unable to find Texture2D with Name: {}", TextureName);
		return *s_NameToTexture2DLibrary[TextureName];
	}

	Ref<TextureCube> TextureLibrary::GetCubeFromID(uint32_t ID)
	{
		YGG_ASSERT(s_IdToNameLibrary.find(ID) != s_IdToNameLibrary.end(),
			"Unable to find TextureCube with ID: {}", ID);
		const auto TextureName = s_IdToNameLibrary[ID];
		YGG_ASSERT(s_NameToTexture2DLibrary.find(TextureName) != s_NameToTexture2DLibrary.end(),
			"Unable to find TextureCube with Name: {}", TextureName);
		return s_NameToTextureCubeLibrary[TextureName];
	}

	uint32_t TextureLibrary::TryGetCubeIDFromName(const std::string& Name)
	{
		if (HasCube(Name))
			return GetIDFromName(Name);
		YGG_LOG_WARN(
			"Unable to find TextureCube with name: '{}'.  Returning Black TextureCube ID instead.");
		return GetIDFromName("Black TextureCube");
	}

	uint32_t TextureLibrary::TryGet2DIDFromName(const std::string& Name, bool FallBackIsWhiteTexture)
	{
		if (Has2D(Name))
			return GetIDFromName(Name);
		YGG_LOG_WARN("Unable to find TextureCube with name: '{}'.  Returning {} ID instead.",
			FallBackIsWhiteTexture ? "White Texture" : "Black Texture");
		return GetIDFromName("Black TextureCube");
	}

	uint32_t TextureLibrary::TryGet2DArrayIDFromName(const std::string& Name)
	{
		if (Has2D(Name))
			return GetIDFromName(Name);
		YGG_LOG_WARN(
			"Unable to find TextureCube with name: '{}'.  Returning White Texture2DArray ID instead.");
		return GetIDFromName("White Texture2DArray");
	}

	bool TextureLibrary::Has2DArray(const std::string& Name)
	{
		return s_NameToTexture2DArrayLibrary.find(Name) != s_NameToTexture2DArrayLibrary.end();
	}

	Ref<Texture2DArray> TextureLibrary::LoadTexture2DArray(const Texture2DArraySpecification& Spec)
	{
		if (Has2DArray(Spec.Name))
			return s_NameToTexture2DArrayLibrary[Spec.Name];

		Ref<Texture2DArray> texture = CreateRef<Texture2DArray>(Spec);
		AddTexture2DArray(texture);
		return texture;
	}

	void TextureLibrary::AddTexture2DArray(const Ref<Texture2DArray>& texture)
	{
		if (Has2DArray(texture->GetName()))
		{
			YGG_LOG_TRACE("Texture2DArray with name '{}' already contained in Texture Library.",
				texture->GetName());
			return;
		}

		s_NameToTexture2DArrayLibrary[texture->GetName()] = texture;
		s_IdToNameLibrary[texture->GetID()] = texture->GetName();
		s_NameToIDLibrary[texture->GetName()] = texture->GetID();
		YGG_LOG_TRACE("Added Texture2DArray with name: '{}' to the Texture Library.", texture->GetName());
	}

	Ref<Texture2D> TextureLibrary::LoadTexture2D(const std::string& filePath)
	{
		Texture2DSpecification defaultFromFileSpec = {
			ImageUtils::WrapMode::Repeat,
			ImageUtils::WrapMode::Repeat,
			ImageUtils::FilterMode::LinearMipLinear,
			ImageUtils::FilterMode::Linear,
			ImageUtils::ImageInternalFormat::FromImage,
			ImageUtils::ImageDataLayout::FromImage,
			ImageUtils::ImageDataType::UByte,
		};

		Ref<Texture2D> texture = CreateRef<Texture2D>(filePath, defaultFromFileSpec);
		AddTexture2D(texture);

		return texture;
	}

	Ref<Texture2D> TextureLibrary::LoadTexture2D(const Texture2DSpecification& Spec, void* Data)
	{
		if (Has2D(Spec.Name))
			return s_NameToTexture2DLibrary[Spec.Name];

		Texture2DSpecification defaultFromFileSpec = {
			ImageUtils::WrapMode::Repeat,
			ImageUtils::WrapMode::Repeat,
			ImageUtils::FilterMode::LinearMipLinear,
			ImageUtils::FilterMode::Linear,
			ImageUtils::ImageInternalFormat::FromImage,
			ImageUtils::ImageDataLayout::FromImage,
			ImageUtils::ImageDataType::UByte,
		};

		Ref<Texture2D> texture = CreateRef<Texture2D>(Spec, Data);
		AddTexture2D(texture);

		return texture;
	}

	const Ref<Texture2D>& TextureLibrary::Get2D(const std::string& name)
	{
		YGG_ASSERT(Has2D(name), "No Texture2D with name '{}' found in Texture Library.", name)
		return s_NameToTexture2DLibrary.at(name);
	}

	const Ref<TextureCube>& TextureLibrary::GetCube(const std::string& name)
	{
		YGG_ASSERT(HasCube(name), "No TextureCube with name '{}' found in Texture Library.", name)
		return s_NameToTextureCubeLibrary.at(name);
	}

	const Ref<Texture2DArray>& TextureLibrary::Get2DArray(const std::string& name)
	{
		YGG_ASSERT(Has2DArray(name), "No Texture2DArray with name '{}' found in Texture Library.", name)
		return s_NameToTexture2DArrayLibrary.at(name);
	}

	const Ref<Texture2DArray>& TextureLibrary::TryGet2DArray(const std::string& name)
	{
		return Has2DArray(name) ? Get2DArray(name) : Get2DArray("White Texture2DArray");
	}

	const Ref<TextureCube>& TextureLibrary::TryGetCube(const std::string& name)
	{
		return HasCube(name) ? GetCube(name) : GetCube("Black TextureCube");
	}

	const Ref<Texture2D>& TextureLibrary::TryGet2D(const std::string& name,
		bool														  FallBackIsWhiteTexture)
	{
		return Has2D(name)			 ? Get2D(name)
			: FallBackIsWhiteTexture ? Get2D("White Texture")
									 : Get2D("Black Texture2D");
	}

	void TextureLibrary::BindTexture2DToSlot(const std::string& TwoDimensionTextureName,
		uint32_t												Slot)
	{
		YGG_ASSERT(Has2D(TwoDimensionTextureName),
			"TextureLibrary: Unable to bind Texture2D with name '{}' to slot '{}'.  This texture "
			"has not been registered.",
			TwoDimensionTextureName, Slot);
		const auto& Texture2D = s_NameToTexture2DLibrary[TwoDimensionTextureName];
		glBindTextureUnit(Slot, Texture2D->GetID());
	}

	void TextureLibrary::BindTextureCubeToSlot(const std::string& CubeTextureName, uint32_t Slot)
	{
		YGG_ASSERT(HasCube(CubeTextureName),
			"TextureLibrary: Unable to bind TextureCube with name '{}' to slot '{}'.  This "
			"texture has not been registered.",
			CubeTextureName, Slot);
		const auto& TextureCube = s_NameToTextureCubeLibrary[CubeTextureName];
		glBindTextureUnit(Slot, TextureCube->GetID());
	}

	void TextureLibrary::BindTextureToSlot(uint32_t TexID, uint32_t Slot)
	{
		glBindTextureUnit(Slot, TexID);
	}

	std::string TextureLibrary::GetNameFromID(uint32_t TextureID)
	{
		YGG_ASSERT(s_IdToNameLibrary.find(TextureID) != s_IdToNameLibrary.end(),
			"TextureLibrary: Unable to find texture with ID '{}'.", TextureID);
		return s_IdToNameLibrary[TextureID];
	}

	uint32_t TextureLibrary::GetIDFromName(const std::string& Name)
	{
		YGG_ASSERT(s_NameToIDLibrary.find(Name) != s_NameToIDLibrary.end(),
			"TextureLibrary: Unable to find texture with name '{}'.", Name);
		return s_NameToIDLibrary[Name];
	}

	void TextureLibrary::LoadWhiteTexture()
	{
		Texture2DSpecification whiteTextureSpec = { askygg::ImageUtils::WrapMode::Repeat,
			askygg::ImageUtils::WrapMode::Repeat,
			askygg::ImageUtils::FilterMode::Linear,
			askygg::ImageUtils::FilterMode::Linear,
			askygg::ImageUtils::ImageInternalFormat::RGBA8,
			askygg::ImageUtils::ImageDataLayout::RGBA,
			askygg::ImageUtils::ImageDataType::UByte,
			1,
			1,
			"White Texture" };

		Ref<Texture2D> WhiteTexture = askygg::CreateRef<askygg::Texture2D>(whiteTextureSpec);
		uint32_t	   whiteTextureData = 0xffffffff;
		WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
		AddTexture2D(WhiteTexture);
	}

	void TextureLibrary::LoadBlackTexture()
	{
		Texture2DSpecification whiteTextureSpec = { askygg::ImageUtils::WrapMode::Repeat,
			askygg::ImageUtils::WrapMode::Repeat,
			askygg::ImageUtils::FilterMode::Linear,
			askygg::ImageUtils::FilterMode::Linear,
			askygg::ImageUtils::ImageInternalFormat::RGBA8,
			askygg::ImageUtils::ImageDataLayout::RGBA,
			askygg::ImageUtils::ImageDataType::UByte,
			1,
			1,
			"Black Texture2D" };

		const Ref<Texture2D> BlackTexture = askygg::CreateRef<askygg::Texture2D>(whiteTextureSpec);
		uint32_t			 blackTextureData = 0xff000000;
		BlackTexture->SetData(&blackTextureData, sizeof(uint32_t));
		AddTexture2D(BlackTexture);
	}

	void TextureLibrary::LoadBlackTextureCube()
	{
		const TextureCubeSpecification BlackCubeTextureSpec = { ImageUtils::WrapMode::ClampToEdge,
			ImageUtils::WrapMode::ClampToEdge,
			ImageUtils::WrapMode::ClampToEdge,
			ImageUtils::FilterMode::LinearMipLinear,
			ImageUtils::FilterMode::Linear,
			ImageUtils::ImageInternalFormat::RGBA8,
			ImageUtils::ImageDataLayout::RGBA,
			ImageUtils::ImageDataType::UByte,
			1,
			"Black TextureCube" };

		const Ref<TextureCube> BlackTextureCube = CreateRef<TextureCube>(BlackCubeTextureSpec);
		uint32_t			   blackTextureData = 0xff000000;
		BlackTextureCube->SetData(&blackTextureData, sizeof(uint32_t));
		AddTextureCube(BlackTextureCube);
	}

	void TextureLibrary::LoadWhiteTextureArray()
	{
		const Texture2DArraySpecification SingleLayerWhiteTextureSpec = {
			ImageUtils::WrapMode::ClampToEdge,
			ImageUtils::WrapMode::ClampToEdge,
			ImageUtils::FilterMode::LinearMipLinear,
			ImageUtils::FilterMode::Linear,
			ImageUtils::ImageInternalFormat::RGBA8,
			ImageUtils::ImageDataLayout::RGBA,
			ImageUtils::ImageDataType::UByte,
			1,
			1,
			1,
			"White Texture2DArray"
		};

		const Ref<Texture2DArray> WhiteTexture2DArray =
			CreateRef<Texture2DArray>(SingleLayerWhiteTextureSpec);
		uint32_t whiteTextureData = 0xffffffff;
		WhiteTexture2DArray->SetDataAllLayers(&whiteTextureData, sizeof(uint32_t));
		AddTexture2DArray(WhiteTexture2DArray);
	}

	std::unordered_map<std::string, Ref<Texture2D>>		 TextureLibrary::s_NameToTexture2DLibrary;
	std::unordered_map<std::string, Ref<TextureCube>>	 TextureLibrary::s_NameToTextureCubeLibrary;
	std::unordered_map<std::string, Ref<Texture2DArray>> TextureLibrary::s_NameToTexture2DArrayLibrary;
	std::unordered_map<uint32_t, std::string>			 TextureLibrary::s_IdToNameLibrary;
	std::unordered_map<std::string, uint32_t>			 TextureLibrary::s_NameToIDLibrary;
} // namespace askygg
