#pragma once

#include "askygg/core/buffer.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

#include <string>
#include <memory>
#include <vulkan/vulkan.h>
#include <glm/gtc/integer.hpp>
#include <map>

namespace askygg
{
	enum class ImageFormat
	{
		None = 0,
		RED8UN,
		RED8UI,
		RED16UI,
		RED32UI,
		RED32F,
		RG8,
		RG16F,
		RG32F,
		RGB,
		RGBA,
		RGBA16F,
		RGBA32F,

		B10R11G11UF,

		SRGB,

		DEPTH32FSTENCIL8UINT,
		DEPTH32F,
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8,
	};

	enum class ImageUsage
	{
		None = 0,
		Texture,
		Attachment,
		Storage,
		HostRead
	};

	enum class TextureWrap
	{
		None = 0,
		Clamp,
		Repeat
	};

	enum class TextureFilter
	{
		None = 0,
		Linear,
		Nearest,
		Cubic
	};

	enum class TextureType
	{
		None = 0,
		Texture2D,
		TextureCube
	};

	struct ImageSpecification
	{
		std::string DebugName;

		ImageFormat			  Format = ImageFormat::RGBA;
		ImageUsage			  Usage = ImageUsage::Texture;
		VkMemoryPropertyFlags Properties;
		bool				  UsedInTransferOps = false;
		uint32_t			  Width = 1;
		uint32_t			  Height = 1;
		uint32_t			  Mips = 1;
		uint32_t			  Layers = 1;
		bool				  CreateSampler = true;
	};

	struct VulkanImageInfo
	{
		VkImage		   Image = nullptr;
		VkImageView	   ImageView = nullptr;
		VkSampler	   Sampler = nullptr;
		VkDeviceMemory m_TextureImageMemory = nullptr;
	};

	class VulkanImage2D
	{
	public:
		explicit VulkanImage2D(VulkanDevice& deviceRef, ImageSpecification specification);
		~VulkanImage2D();

		void Resize(uint32_t width, uint32_t height);
		void Invalidate();
		void Release();
		void UpdateDescriptor();

		VkImageView GetMipImageView(uint32_t mip);

		[[nodiscard]] uint32_t GetWidth() const { return m_Specification.Width; }
		[[nodiscard]] uint32_t GetHeight() const { return m_Specification.Height; }
		[[nodiscard]] float	   GetAspectRatio() const { return static_cast<float>(m_Specification.Width) / static_cast<float>(m_Specification.Height); }

		VulkanImageInfo&					 GetImageInfo() { return m_Info; }
		[[nodiscard]] const VulkanImageInfo& GetImageInfo() const { return m_Info; }
		[[nodiscard]] VkDescriptorImageInfo& GetDescriptorInfo() { return m_DescriptorImageInfo; }

		ImageSpecification&	 GetSpecification() { return m_Specification; }
		[[nodiscard]] Buffer GetBuffer() const { return m_ImageData; }
		Buffer&				 GetBuffer() { return m_ImageData; }

	private:
		VulkanDevice&	   m_DeviceRef;
		ImageSpecification m_Specification;
		BufferSafe		   m_ImageData;

		std::vector<VkImageView>		m_LayerImageViews;
		std::map<uint32_t, VkImageView> m_MipImageViews;
		VulkanImageInfo					m_Info{};
		VkDescriptorImageInfo			m_DescriptorImageInfo{};
	};

	namespace ImageUtils
	{
		inline uint32_t GetImageFormatBPP(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RED8UN:
					return 1;
				case ImageFormat::RED8UI:
					return 1;
				case ImageFormat::RED16UI:
					return 2;
				case ImageFormat::RED32UI:
					return 4;
				case ImageFormat::RED32F:
					return 4;
				case ImageFormat::RGB:
				case ImageFormat::SRGB:
					return 3;
				case ImageFormat::RGBA:
					return 4;
				case ImageFormat::RGBA16F:
					return 2 * 4;
				case ImageFormat::RGBA32F:
					return 4 * 4;
				case ImageFormat::B10R11G11UF:
					return 4;
			}
			assert(false);
		}

		inline bool IsIntegerBased(const ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RED16UI:
				case ImageFormat::RED32UI:
				case ImageFormat::RED8UI:
				case ImageFormat::DEPTH32FSTENCIL8UINT:
					return true;
				case ImageFormat::DEPTH32F:
				case ImageFormat::RED8UN:
				case ImageFormat::RGBA32F:
				case ImageFormat::B10R11G11UF:
				case ImageFormat::RG16F:
				case ImageFormat::RG32F:
				case ImageFormat::RED32F:
				case ImageFormat::RG8:
				case ImageFormat::RGBA:
				case ImageFormat::RGBA16F:
				case ImageFormat::RGB:
				case ImageFormat::SRGB:
				case ImageFormat::DEPTH24STENCIL8:
					return false;
			}
			assert(false);
		}

		inline VkFormat VulkanImageFormat(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::RED8UN:
					return VK_FORMAT_R8_UNORM;
				case ImageFormat::RED8UI:
					return VK_FORMAT_R8_UINT;
				case ImageFormat::RED16UI:
					return VK_FORMAT_R16_UINT;
				case ImageFormat::RED32UI:
					return VK_FORMAT_R32_UINT;
				case ImageFormat::RED32F:
					return VK_FORMAT_R32_SFLOAT;
				case ImageFormat::RG8:
					return VK_FORMAT_R8G8_UNORM;
				case ImageFormat::RG16F:
					return VK_FORMAT_R16G16_SFLOAT;
				case ImageFormat::RG32F:
					return VK_FORMAT_R32G32_SFLOAT;
				case ImageFormat::RGBA:
					return VK_FORMAT_R8G8B8A8_UNORM;
				case ImageFormat::RGBA16F:
					return VK_FORMAT_R16G16B16A16_SFLOAT;
				case ImageFormat::RGBA32F:
					return VK_FORMAT_R32G32B32A32_SFLOAT;
				case ImageFormat::B10R11G11UF:
					return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
				case ImageFormat::DEPTH32FSTENCIL8UINT:
					return VK_FORMAT_D32_SFLOAT_S8_UINT;
				case ImageFormat::DEPTH32F:
					return VK_FORMAT_D32_SFLOAT;
				case ImageFormat::DEPTH24STENCIL8:
					return VK_FORMAT_D32_SFLOAT; // TODO:: Use device depth format.
			}
			assert(false);
			return VK_FORMAT_UNDEFINED;
		}

		inline uint32_t CalculateMipCount(uint32_t width, uint32_t height)
		{
			return (uint32_t)glm::floor(glm::log2(glm::min(width, height))) + 1;
		}

		inline uint32_t GetImageMemorySize(ImageFormat format, uint32_t width, uint32_t height)
		{
			return width * height * GetImageFormatBPP(format);
		}

		inline bool IsDepthFormat(ImageFormat format)
		{
			if (format == ImageFormat::DEPTH24STENCIL8 || format == ImageFormat::DEPTH32F || format == ImageFormat::DEPTH32FSTENCIL8UINT)
				return true;

			return false;
		}

		void InsertImageMemoryBarrier(
			VkCommandBuffer			cmdbuffer,
			VkImage					image,
			VkAccessFlags			srcAccessMask,
			VkAccessFlags			dstAccessMask,
			VkImageLayout			oldImageLayout,
			VkImageLayout			newImageLayout,
			VkPipelineStageFlags	srcStageMask,
			VkPipelineStageFlags	dstStageMask,
			VkImageSubresourceRange subresourceRange);

		void SetImageLayout(
			VkCommandBuffer			cmdbuffer,
			VkImage					image,
			VkImageLayout			oldImageLayout,
			VkImageLayout			newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags	srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags	dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

		void SetImageLayout(
			VkCommandBuffer		 cmdbuffer,
			VkImage				 image,
			VkImageAspectFlags	 aspectMask,
			VkImageLayout		 oldImageLayout,
			VkImageLayout		 newImageLayout,
			VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	}

struct VulkanImageViewSpecification
	{
		std::shared_ptr<VulkanImage2D> Image;
		uint32_t					   Mip = 0;
		std::string					   DebugName;
	};

	class VulkanImageView
	{
	public:
		explicit VulkanImageView(VulkanDevice& deviceRef, VulkanImageViewSpecification specification);
		~VulkanImageView();

		void								Invalidate();
		[[nodiscard]] VkImageView			GetImageView() const { return m_ImageView; }
		[[nodiscard]] VkDescriptorImageInfo GetDescriptorInfo() const { return m_DescriptorImageInfo; }

	private:
		VulkanImageViewSpecification m_Specification;
		VkImageView					 m_ImageView = nullptr;

		VulkanDevice&		  m_DeviceRef;
		VkDescriptorImageInfo m_DescriptorImageInfo{};
	};
}
