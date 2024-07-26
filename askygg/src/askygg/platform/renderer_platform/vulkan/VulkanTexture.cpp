#include "VulkanTexture.h"
#include "vulkan_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>
#include <utility>

namespace askygg
{
	namespace TextureUtils
	{
		static VkSamplerAddressMode VulkanSamplerWrap(TextureWrap wrap)
		{
			switch (wrap)
			{
				case TextureWrap::Clamp:
					return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				case TextureWrap::Repeat:
					return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}
			assert(false && "Unknown wrap mode");
			return (VkSamplerAddressMode)0;
		}

		static VkFilter VulkanSamplerFilter(TextureFilter filter)
		{
			switch (filter)
			{
				case TextureFilter::Linear:
					return VK_FILTER_LINEAR;
				case TextureFilter::Nearest:
					return VK_FILTER_NEAREST;
				case TextureFilter::Cubic:
					return VK_FILTER_CUBIC_IMG;
			}
			assert(false && "Unknown filter");
			return (VkFilter)0;
		}

		static size_t GetMemorySize(ImageFormat format, uint32_t width, uint32_t height)
		{
			switch (format)
			{
				case ImageFormat::RED16UI:
					return width * height * sizeof(uint16_t);
				case ImageFormat::RG16F:
					return width * height * 2 * sizeof(uint16_t);
				case ImageFormat::RG32F:
					return width * height * 2 * sizeof(float);
				case ImageFormat::RED32F:
					return width * height * sizeof(float);
				case ImageFormat::RED8UN:
					return width * height;
				case ImageFormat::RED8UI:
					return width * height;
				case ImageFormat::RGBA:
					return width * height * 4;
				case ImageFormat::RGBA32F:
					return width * height * 4 * sizeof(float);
				case ImageFormat::B10R11G11UF:
					return width * height * sizeof(float);
			}
			assert(false);
			return 0;
		}

		Buffer ToBufferFromFile(const std::string& path, ImageFormat& outFormat, uint32_t& outWidth, uint32_t& outHeight)
		{
			Buffer imageBuffer;

			int width, height, channels;
			if (stbi_is_hdr(path.c_str()))
			{
				imageBuffer.Data = (byte*)stbi_loadf(path.c_str(), &width, &height, &channels, 4);
				imageBuffer.Size = width * height * 4 * sizeof(float);
				outFormat = ImageFormat::RGBA32F;
			}
			else
			{
				imageBuffer.Data = stbi_load(path.c_str(), &width, &height, &channels, 4);
				imageBuffer.Size = width * height * 4;
				outFormat = ImageFormat::RGBA;
			}

			if (!imageBuffer.Data)
				return {};

			outWidth = width;
			outHeight = height;
			return imageBuffer;
		}

		Buffer ToBufferFromMemory(Buffer buffer, ImageFormat& outFormat, uint32_t& outWidth, uint32_t& outHeight)
		{
			Buffer imageBuffer;

			int width, height, channels;
			if (stbi_is_hdr_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size))
			{
				imageBuffer.Data = (byte*)stbi_loadf_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size, &width, &height, &channels, STBI_rgb_alpha);
				imageBuffer.Size = width * height * 4 * sizeof(float);
				outFormat = ImageFormat::RGBA32F;
			}
			else
			{
				imageBuffer.Data = stbi_load_from_memory((const stbi_uc*)buffer.Data, (int)buffer.Size, &width, &height, &channels, STBI_rgb_alpha);
				imageBuffer.Size = width * height * 4;
				outFormat = ImageFormat::RGBA;
			}

			if (!imageBuffer.Data)
				return {};

			outWidth = width;
			outHeight = height;
			return imageBuffer;
		}

	}


VulkanTexture2D::VulkanTexture2D(VulkanDevice& deviceRef, TextureSpecification specification, const std::string& filepath)
		: m_DeviceRef(deviceRef), m_Specification(std::move(specification)), m_Path(filepath)
	{
		m_ImageData = TextureUtils::ToBufferFromFile(filepath, m_Specification.Format, m_Specification.Width, m_Specification.Height);
		assert(m_ImageData && "Failed to create Image Buffer from file.");

		ImageSpecification spec;
		spec.Format = m_Specification.Format;
		spec.Width = m_Specification.Width;
		spec.Height = m_Specification.Height;
		spec.Mips = specification.GenerateMips ? GetMipLevelCount() : 1;
		spec.DebugName = specification.DebugName;
		spec.CreateSampler = false;
		m_Image = std::make_shared<VulkanImage2D>(deviceRef, spec);
		Invalidate();
	}

	VulkanTexture2D::VulkanTexture2D(VulkanDevice& deviceRef, TextureSpecification specification, Buffer data)
		: m_DeviceRef(deviceRef), m_Specification(std::move(specification))
	{
		if (m_Specification.Height == 0)
		{
			m_ImageData = TextureUtils::ToBufferFromMemory(
				Buffer(data.Data, m_Specification.Width),
				m_Specification.Format,
				m_Specification.Width, m_Specification.Height);

			assert(m_ImageData && "Failed to create Image Buffer from input Buffer.");
		}
		else if (data)
		{
			auto size = static_cast<uint32_t>(TextureUtils::GetMemorySize(m_Specification.Format, m_Specification.Width, m_Specification.Height));
			m_ImageData = Buffer::Copy(data.Data, size);
		}
		else
		{
			auto size = static_cast<uint32_t>(TextureUtils::GetMemorySize(m_Specification.Format, m_Specification.Width, m_Specification.Height));
			m_ImageData.Allocate(size);
			m_ImageData.ZeroInitialize();
		}

		ImageSpecification spec;
		spec.Format = m_Specification.Format;
		spec.Width = m_Specification.Width;
		spec.Height = m_Specification.Height;
		spec.Mips = specification.GenerateMips ? VulkanTexture2D::GetMipLevelCount() : 1;
		spec.DebugName = specification.DebugName;
		spec.CreateSampler = false;
		if (specification.Storage)
			spec.Usage = ImageUsage::Storage;
		m_Image = std::make_shared<VulkanImage2D>(deviceRef, spec);
		Invalidate();
	}

	VulkanTexture2D::~VulkanTexture2D()
	{
		if (m_Image)
			m_Image->Release();
		m_ImageData.Release();
	}

	void VulkanTexture2D::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	void VulkanTexture2D::Invalidate()
	{
		m_Image->Release();

		uint32_t mipCount = m_Specification.GenerateMips ? GetMipLevelCount() : 1;

		ImageSpecification& imageSpec = m_Image->GetSpecification();
		imageSpec.Format = m_Specification.Format;
		imageSpec.Width = m_Specification.Width;
		imageSpec.Height = m_Specification.Height;
		imageSpec.Mips = mipCount;
		imageSpec.CreateSampler = false;
		if (!m_ImageData)
			imageSpec.Usage = ImageUsage::Storage;

		m_Image->Invalidate();
		auto& info = m_Image->GetImageInfo();

		if (m_ImageData)
		{
			VkDeviceSize imageSize = m_ImageData.Size;

			VkMemoryAllocateInfo memAllocInfo{};
			memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

			VkBuffer	   stagingBuffer;
			VkDeviceMemory stagingBufferMemory;

			m_DeviceRef.CreateBuffer(
				imageSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer,
				stagingBufferMemory);

			void* data;
			vkMapMemory(m_DeviceRef.GetDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
			memcpy(data, m_ImageData.Data, static_cast<size_t>(imageSize));
			vkUnmapMemory(m_DeviceRef.GetDevice(), stagingBufferMemory);

			VkCommandBuffer copyCommand = m_DeviceRef.BeginSingleTimeCommands();

			// The sub resource range describes the regions of the image that will be transitioned using the memory barriers below
			VkImageSubresourceRange subresourceRange = {};
			// Image only contains color data
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			// Start at first mip level
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = 1;

			// Insert a memory dependency at the proper pipeline stages that will execute the image layout transition
			// Source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
			// Destination pipeline stage is copy command execution (VK_PIPELINE_STAGE_TRANSFER_BIT)
			ImageUtils::SetImageLayout(
				copyCommand,
				info.Image,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				subresourceRange,
				VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = 0;
			bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = m_Specification.Width;
			bufferCopyRegion.imageExtent.height = m_Specification.Height;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = 0;

			// Copy mip levels from staging buffer
			vkCmdCopyBufferToImage(
				copyCommand,
				stagingBuffer,
				info.Image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&bufferCopyRegion);

			if (mipCount > 1) // Mips to generate
			{
				ImageUtils::InsertImageMemoryBarrier(copyCommand, info.Image,
					VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
					subresourceRange);
			}
			else
			{
				ImageUtils::InsertImageMemoryBarrier(copyCommand, info.Image,
					VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
					VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_Image->GetDescriptorInfo().imageLayout,
					VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
					subresourceRange);
			}

			m_DeviceRef.EndSingleTimeCommand(copyCommand);
			vkDestroyBuffer(m_DeviceRef.GetDevice(), stagingBuffer, nullptr);
			vkFreeMemory(m_DeviceRef.GetDevice(), stagingBufferMemory, nullptr);
		}
		else
		{
			VkCommandBuffer			transitionCommandBuffer = m_DeviceRef.BeginSingleTimeCommands();
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.layerCount = 1;
			subresourceRange.levelCount = mipCount;
			ImageUtils::SetImageLayout(
				transitionCommandBuffer,
				info.Image,
				VK_IMAGE_LAYOUT_UNDEFINED, m_Image->GetDescriptorInfo().imageLayout,
				subresourceRange);
			m_DeviceRef.EndSingleTimeCommand(transitionCommandBuffer);
		}

		// Create a texture sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = TextureUtils::VulkanSamplerFilter(m_Specification.SamplerFilter);
		samplerInfo.minFilter = TextureUtils::VulkanSamplerFilter(m_Specification.SamplerFilter);
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = TextureUtils::VulkanSamplerWrap(m_Specification.SamplerWrap);
		samplerInfo.addressModeV = TextureUtils::VulkanSamplerWrap(m_Specification.SamplerWrap);
		samplerInfo.addressModeW = TextureUtils::VulkanSamplerWrap(m_Specification.SamplerWrap);
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(mipCount);

		// Enable anisotropic filtering
		// This feature is optional, so we must check if it's supported on the device
		samplerInfo.maxAnisotropy = 16.0f;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VK_CHECK_RESULT(vkCreateSampler(m_DeviceRef.GetDevice(), &samplerInfo, nullptr, &info.Sampler));
		m_Image->UpdateDescriptor();

		if (!m_Specification.Storage)
		{
			VkImageViewCreateInfo view{};
			view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view.format = ImageUtils::VulkanImageFormat(m_Specification.Format);
			view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			// The subresource range describes the set of mip levels (and array layers) that can be accessed through this image view
			// It's possible to create multiple image views for a single image referring to different (and/or overlapping) ranges of the image
			view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view.subresourceRange.baseMipLevel = 0;
			view.subresourceRange.baseArrayLayer = 0;
			view.subresourceRange.layerCount = 1;
			view.subresourceRange.levelCount = mipCount;
			view.image = info.Image;
			VK_CHECK_RESULT(vkCreateImageView(m_DeviceRef.GetDevice(), &view, nullptr, &info.ImageView));

			m_Image->UpdateDescriptor();
		}

		if (m_ImageData && m_Specification.GenerateMips && mipCount > 1)
			GenerateMips();

		m_ImageData.Release();
		m_ImageData = Buffer();
	}

	uint32_t VulkanTexture2D::GetMipLevelCount() const
	{
		return ImageUtils::CalculateMipCount(m_Specification.Width, m_Specification.Height);
	}

	glm::uvec2 VulkanTexture2D::GetMipSize(uint32_t mip) const
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

	void VulkanTexture2D::GenerateMips()
	{
		const auto& imageInfo = m_Image->GetImageInfo();

		VkCommandBuffer blitCmd = m_DeviceRef.BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = imageInfo.Image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		uint32_t mipLevels = GetMipLevelCount();
		for (uint32_t i = 1; i < mipLevels; i++)
		{
			VkImageBlit imageBlit{};

			// Source
			imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcOffsets[1].x = int32_t(m_Specification.Width >> (i - 1));
			imageBlit.srcOffsets[1].y = int32_t(m_Specification.Height >> (i - 1));
			imageBlit.srcOffsets[1].z = 1;

			// Destination
			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstOffsets[1].x = int32_t(m_Specification.Width >> i);
			imageBlit.dstOffsets[1].y = int32_t(m_Specification.Height >> i);
			imageBlit.dstOffsets[1].z = 1;

			VkImageSubresourceRange mipSubRange = {};
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = 1;

			// Prepare current mip level as image blit destination
			ImageUtils::InsertImageMemoryBarrier(blitCmd, imageInfo.Image,
				0, VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				mipSubRange);

			// Blit from previous level
			vkCmdBlitImage(
				blitCmd,
				imageInfo.Image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				imageInfo.Image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&imageBlit,
				TextureUtils::VulkanSamplerFilter(m_Specification.SamplerFilter));

			// Prepare current mip level as image blit source for next level
			ImageUtils::InsertImageMemoryBarrier(blitCmd, imageInfo.Image,
				VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				mipSubRange);
		}

		// After the loop, all mip layers are in TRANSFER_SRC layout, so transition all to SHADER_READ
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.layerCount = 1;
		subresourceRange.levelCount = mipLevels;

		ImageUtils::InsertImageMemoryBarrier(blitCmd, imageInfo.Image,
			VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			subresourceRange);

		m_DeviceRef.EndSingleTimeCommand(blitCmd);
	}
}

