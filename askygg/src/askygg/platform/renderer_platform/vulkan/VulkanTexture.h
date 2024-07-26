#pragma once

#include "VulkanImage.h"
#include "VulkanDevice.h"
#include "askygg/core/Buffer.h"

#include <glm/glm.hpp>
#include <memory>
#include <filesystem>
#include <vulkan/vulkan.h>

namespace askygg
{
	struct TextureSpecification
	{
		ImageFormat	  Format = ImageFormat::RGBA;
		uint32_t	  Width = 1;
		uint32_t	  Height = 1;
		TextureWrap	  SamplerWrap = TextureWrap::Repeat;
		TextureFilter SamplerFilter = TextureFilter::Linear;

		bool		GenerateMips = true;
		bool		Storage = false;
		std::string DebugName;
	};

	class VulkanTexture2D
	{
	public:
		VulkanTexture2D(VulkanDevice& deviceRef, TextureSpecification specification, const std::string& filepath);
		explicit VulkanTexture2D(VulkanDevice& deviceRef, TextureSpecification specification, Buffer data = Buffer());
		~VulkanTexture2D();

		void Invalidate();
		void Resize(uint32_t width, uint32_t height);
		void GenerateMips();

		[[nodiscard]] ImageFormat GetFormat() const { return m_Specification.Format; }
		[[nodiscard]] uint32_t	  GetWidth() const { return m_Specification.Width; }
		[[nodiscard]] uint32_t	  GetHeight() const { return m_Specification.Height; }
		[[nodiscard]] glm::uvec2  GetSize() const { return { m_Specification.Width, m_Specification.Height }; }

		[[nodiscard]] std::shared_ptr<VulkanImage2D> GetImage() const { return m_Image; }
		[[nodiscard]] VkDescriptorImageInfo&		 GetDescriptorInfo() { return m_Image->GetDescriptorInfo(); }

		[[nodiscard]] uint32_t	 GetMipLevelCount() const;
		[[nodiscard]] glm::uvec2 GetMipSize(uint32_t mip) const;

	private:
		VulkanDevice&		 m_DeviceRef;
		TextureSpecification m_Specification;
		std::string			 m_Path;

		Buffer						   m_ImageData;
		std::shared_ptr<VulkanImage2D> m_Image;
	};
}

