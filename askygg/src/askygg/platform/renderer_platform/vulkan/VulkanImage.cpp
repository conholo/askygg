#include "VulkanImage.h"
#include "VulkanUtils.h"

#include <utility>

namespace askygg
{
	VulkanImage2D::VulkanImage2D(VulkanDevice& deviceRef, ImageSpecification specification)
		: m_DeviceRef(deviceRef), m_Specification(std::move(specification))
	{
		assert(m_Specification.Width > 0 && m_Specification.Height > 0);
	}

	VulkanImage2D::~VulkanImage2D()
	{
		Release();
	}

	void VulkanImage2D::Release()
	{
		if (m_Info.Image == nullptr)
			return;

		vkDestroyImageView(m_DeviceRef.GetDevice(), m_Info.ImageView, nullptr);
		vkDestroySampler(m_DeviceRef.GetDevice(), m_Info.Sampler, nullptr);

		for (auto& view : m_MipImageViews)
		{
			if (view.second)
			{
				vkDestroyImageView(m_DeviceRef.GetDevice(), view.second, nullptr);
			}
		}

		for (auto& view : m_LayerImageViews)
		{
			if (view)
			{
				vkDestroyImageView(m_DeviceRef.GetDevice(), view, nullptr);
			}
		}

		vkDestroyImage(m_DeviceRef.GetDevice(), m_Info.Image, nullptr);
		vkFreeMemory(m_DeviceRef.GetDevice(), m_Info.m_TextureImageMemory, nullptr);

		m_Info.Image = nullptr;
		m_Info.ImageView = nullptr;
		if (m_Specification.CreateSampler)
			m_Info.Sampler = nullptr;
		m_LayerImageViews.clear();
		m_MipImageViews.clear();
	}

	void VulkanImage2D::Invalidate()
	{
		assert(m_Specification.Width > 0 && m_Specification.Height > 0);

		Release();

		VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
		if (m_Specification.Usage == ImageUsage::Attachment)
		{
			if (ImageUtils::IsDepthFormat(m_Specification.Format))
				usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			else
				usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
		if (m_Specification.UsedInTransferOps || m_Specification.Usage == ImageUsage::Texture)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}
		if (m_Specification.Usage == ImageUsage::Storage)
		{
			usage |= VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VkImageAspectFlags aspectMask = ImageUtils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8)
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		VkFormat vulkanFormat = ImageUtils::VulkanImageFormat(m_Specification.Format);

		VkImageCreateInfo imageCreateInfo = {};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = vulkanFormat;
		imageCreateInfo.extent.width = m_Specification.Width;
		imageCreateInfo.extent.height = m_Specification.Height;
		imageCreateInfo.extent.depth = 1;
		imageCreateInfo.mipLevels = m_Specification.Mips;
		imageCreateInfo.arrayLayers = m_Specification.Layers;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = m_Specification.Usage == ImageUsage::HostRead ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = usage;

		if (m_Specification.Usage == ImageUsage::Storage)
		{
			QueueFamilyIndices indices = m_DeviceRef.FindQueueFamilies(m_DeviceRef.GetPhysicalDevice());

			bool hasComputeAndGraphicsQueues = indices.ComputeFamily.has_value() && indices.GraphicsFamily.has_value();
			if (hasComputeAndGraphicsQueues)
			{
				bool areDifferent = indices.GraphicsFamily.value() != indices.ComputeFamily.value();
				if (areDifferent)
				{
					std::vector<uint32_t> queueFamilyIndices = {
						indices.GraphicsFamily.value(),
						indices.ComputeFamily.value()
					};

					imageCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
					imageCreateInfo.queueFamilyIndexCount = 2;
					imageCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
				}
			}
		}

		m_DeviceRef.CreateImageWithInfo(
			imageCreateInfo,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			m_Info.Image,
			m_Info.m_TextureImageMemory);

		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = m_Specification.Layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = vulkanFormat;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.subresourceRange = {};
		imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = m_Specification.Mips;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = m_Specification.Layers;
		imageViewCreateInfo.image = m_Info.Image;
		VK_CHECK_RESULT(vkCreateImageView(m_DeviceRef.GetDevice(), &imageViewCreateInfo, nullptr, &m_Info.ImageView));

		if (m_Specification.CreateSampler)
		{
			VkSamplerCreateInfo samplerCreateInfo = {};
			samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCreateInfo.anisotropyEnable = VK_TRUE;
			samplerCreateInfo.maxAnisotropy = 16.0f;

			if (ImageUtils::IsIntegerBased(m_Specification.Format))
			{
				samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
				samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
				samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			}
			else
			{
				samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
				samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
				samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			}

			samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
			samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
			samplerCreateInfo.mipLodBias = 0.0f;
			samplerCreateInfo.minLod = 0.0f;
			samplerCreateInfo.maxLod = 100.0f;
			samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			VK_CHECK_RESULT(vkCreateSampler(m_DeviceRef.GetDevice(), &samplerCreateInfo, nullptr, &m_Info.Sampler));
		}

		if (m_Specification.Usage == ImageUsage::Storage)
		{
			// Transition image to GENERAL layout
			VkCommandBuffer commandBuffer = m_DeviceRef.BeginSingleTimeCommands();

			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = m_Specification.Mips;
			subresourceRange.layerCount = m_Specification.Layers;

			ImageUtils::InsertImageMemoryBarrier(commandBuffer, m_Info.Image,
				0, 0,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				subresourceRange);

			m_DeviceRef.EndSingleTimeCommand(commandBuffer);
		}
		else if (m_Specification.Usage == ImageUsage::HostRead)
		{
			// Transition image to TRANSFER_DST layout
			VkCommandBuffer commandBuffer = m_DeviceRef.BeginSingleTimeCommands();

			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = m_Specification.Mips;
			subresourceRange.layerCount = m_Specification.Layers;

			ImageUtils::InsertImageMemoryBarrier(commandBuffer, m_Info.Image,
				0, 0,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				subresourceRange);

			m_DeviceRef.EndSingleTimeCommand(commandBuffer);
		}

		UpdateDescriptor();
	}

	void VulkanImage2D::UpdateDescriptor()
	{
		if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8 || m_Specification.Format == ImageFormat::DEPTH32F || m_Specification.Format == ImageFormat::DEPTH32FSTENCIL8UINT)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		else if (m_Specification.Usage == ImageUsage::Storage)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		else
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if (m_Specification.Usage == ImageUsage::Storage)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		else if (m_Specification.Usage == ImageUsage::HostRead)
			m_DescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		m_DescriptorImageInfo.imageView = m_Info.ImageView;
		m_DescriptorImageInfo.sampler = m_Info.Sampler;
	}

	VkImageView VulkanImage2D::GetMipImageView(uint32_t mip)
	{
		if (m_MipImageViews.find(mip) == m_MipImageViews.end())
		{
			VkImageAspectFlags aspectMask = ImageUtils::IsDepthFormat(m_Specification.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
			if (m_Specification.Format == ImageFormat::DEPTH24STENCIL8)
				aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

			VkFormat vulkanFormat = ImageUtils::VulkanImageFormat(m_Specification.Format);

			VkImageViewCreateInfo imageViewCreateInfo = {};
			imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCreateInfo.format = vulkanFormat;
			imageViewCreateInfo.flags = 0;
			imageViewCreateInfo.subresourceRange = {};
			imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
			imageViewCreateInfo.subresourceRange.baseMipLevel = mip;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;
			imageViewCreateInfo.image = m_Info.Image;

			VK_CHECK_RESULT(vkCreateImageView(m_DeviceRef.GetDevice(), &imageViewCreateInfo, nullptr, &m_MipImageViews[mip]));
			return m_MipImageViews[mip];
		}

		return m_MipImageViews.at(mip);
	}

	void VulkanImage2D::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;
		Invalidate();
	}

	VulkanImageView::VulkanImageView(VulkanDevice& deviceRef, VulkanImageViewSpecification specification)
		: m_DeviceRef(deviceRef), m_Specification(std::move(specification))
	{
		Invalidate();
	}

	VulkanImageView::~VulkanImageView()
	{
		vkDestroyImageView(m_DeviceRef.GetDevice(), m_ImageView, nullptr);
	}

	void VulkanImageView::Invalidate()
	{
		const auto& imageSpec = m_Specification.Image->GetSpecification();

		VkImageAspectFlags aspectMask = ImageUtils::IsDepthFormat(imageSpec.Format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		if (imageSpec.Format == ImageFormat::DEPTH24STENCIL8)
			aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

		VkFormat vulkanFormat = ImageUtils::VulkanImageFormat(imageSpec.Format);

		VkImageViewCreateInfo imageViewCreateInfo{};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.viewType = imageSpec.Layers > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = vulkanFormat;
		imageViewCreateInfo.flags = 0;
		imageViewCreateInfo.subresourceRange = {};
		imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
		imageViewCreateInfo.subresourceRange.baseMipLevel = m_Specification.Mip;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = imageSpec.Layers;
		imageViewCreateInfo.image = m_Specification.Image->GetImageInfo().Image;
		VK_CHECK_RESULT(vkCreateImageView(m_DeviceRef.GetDevice(), &imageViewCreateInfo, nullptr, &m_ImageView));

		m_DescriptorImageInfo = m_Specification.Image->GetDescriptorInfo();
		m_DescriptorImageInfo.imageView = m_ImageView;
	}

	namespace ImageUtils
	{
		void SetImageLayout(
			VkCommandBuffer		 cmdbuffer,
			VkImage				 image,
			VkImageAspectFlags	 aspectMask,
			VkImageLayout		 oldImageLayout,
			VkImageLayout		 newImageLayout,
			VkPipelineStageFlags srcStageMask,
			VkPipelineStageFlags dstStageMask)
		{
			VkImageSubresourceRange subresourceRange = {};
			subresourceRange.aspectMask = aspectMask;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = 1;
			subresourceRange.layerCount = 1;
			SetImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
		}

		void SetImageLayout(
			VkCommandBuffer			cmdbuffer,
			VkImage					image,
			VkImageLayout			oldImageLayout,
			VkImageLayout			newImageLayout,
			VkImageSubresourceRange subresourceRange,
			VkPipelineStageFlags	srcStageMask,
			VkPipelineStageFlags	dstStageMask)
		{
			// Create an image barrier object
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			// Source layouts (old)
			// Source access mask controls actions that have to be finished on the old layout
			// before it will be transitioned to the new layout
			switch (oldImageLayout)
			{
				case VK_IMAGE_LAYOUT_UNDEFINED:
					// Image layout is undefined (or does not matter)
					// Only valid as initial layout
					// No flags required, listed only for completeness
					imageMemoryBarrier.srcAccessMask = 0;
					break;

				case VK_IMAGE_LAYOUT_PREINITIALIZED:
					// Image is preinitialized
					// Only valid as initial layout for linear images, preserves memory contents
					// Make sure host writes have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					// Image is a color attachment
					// Make sure any writes to the color buffer have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					// Image is a depth/stencil attachment
					// Make sure any writes to the depth/stencil buffer have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					// Image is a transfer source
					// Make sure any reads from the image have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					// Image is a transfer destination
					// Make sure any writes to the image have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					// Image is read by a shader
					// Make sure any shader reads from the image have been finished
					imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				default:
					break;
			}

			// Target layouts (new)
			// Destination access mask controls the dependency for the new image layout
			switch (newImageLayout)
			{
				case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
					// Image will be used as a transfer destination
					// Make sure any writes to the image have been finished
					imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
					// Image will be used as a transfer source
					// Make sure any reads from the image have been finished
					imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
					break;

				case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
					// Image will be used as a color attachment
					// Make sure any writes to the color buffer have been finished
					imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
					// Image layout will be used as a depth/stencil attachment
					// Make sure any writes to depth/stencil buffer have been finished
					imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
					break;

				case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
					// Image will be read in a shader (sampler, input attachment)
					// Make sure any writes to the image have been finished
					if (imageMemoryBarrier.srcAccessMask == 0)
					{
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
					}
					imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					break;
				default:
					break;
			}

			// Put barrier inside setup command buffer
			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
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
			VkImageSubresourceRange subresourceRange)
		{
			VkImageMemoryBarrier imageMemoryBarrier{};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

			imageMemoryBarrier.srcAccessMask = srcAccessMask;
			imageMemoryBarrier.dstAccessMask = dstAccessMask;
			imageMemoryBarrier.oldLayout = oldImageLayout;
			imageMemoryBarrier.newLayout = newImageLayout;
			imageMemoryBarrier.image = image;
			imageMemoryBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(
				cmdbuffer,
				srcStageMask,
				dstStageMask,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);
		}
	}

}
