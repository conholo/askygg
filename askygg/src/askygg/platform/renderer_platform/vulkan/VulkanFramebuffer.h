#pragma once

#include "VulkanDevice.h"
#include <glm/fwd.hpp>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace askygg
{
	class VulkanFramebuffer
	{
	public:
		struct Attachment
		{
			struct Specification
			{
				VkFormat			Format{};
				VkImageUsageFlags	Usage{};
				VkAttachmentLoadOp	LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
				VkImageLayout		InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				VkImageLayout		FinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			};

			uint32_t	   Width, Height;
			VkImage		   Image;
			VkDeviceMemory Mem;
			VkImageView	   View;
			Specification  Spec;
		};

		struct ExternalAttachment
		{
			struct Specification
			{
				VkFormat			Format{};
				VkImageUsageFlags	Usage{};
				VkAttachmentLoadOp	LoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				VkAttachmentStoreOp StoreOp = VK_ATTACHMENT_STORE_OP_STORE;
				VkImageLayout		InitialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				VkImageLayout		FinalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			};

			VkImageView	  View;
			VkImage		  Image;
			Specification Spec;
		};

		struct Subpass
		{
			std::vector<VkAttachmentReference> ColorAttachments;
			VkAttachmentReference			   DepthStencilAttachment;
			std::vector<VkAttachmentReference> InputAttachments;
			VkPipelineBindPoint				   BindPoint;
		};

		struct SubpassDependency
		{
			uint32_t			 SrcSubpass;
			uint32_t			 DstSubpass;
			VkPipelineStageFlags SrcStageMask;
			VkPipelineStageFlags DstStageMask;
			VkAccessFlags		 SrcAccessMask;
			VkAccessFlags		 DstAccessMask;
			VkDependencyFlags	 DependencyFlags;
		};

		VulkanFramebuffer(
			VulkanDevice& deviceRef,
			uint32_t width, uint32_t height,
			const std::vector<Attachment::Specification>& attachmentSpecs,
			const std::vector<Subpass>&					  subpasses,
			const std::vector<SubpassDependency>&		  dependencies,
			const std::vector<ExternalAttachment>&		  externalAttachments = {});

		~VulkanFramebuffer();

		static void CreateAttachment(
			VulkanDevice&					 deviceRef,
			const Attachment::Specification& spec,
			Attachment*						 attachment,
			uint32_t width, uint32_t height);

		void Resize(uint32_t width, uint32_t height);

		[[nodiscard]] VkRenderPass	GetRenderPass() const { return m_RenderPass; }
		[[nodiscard]] VkFramebuffer GetFramebuffer() const { return m_Framebuffer; }
		[[nodiscard]] uint32_t		GetWidth() const { return m_Width; }
		[[nodiscard]] uint32_t		GetHeight() const { return m_Height; }

		VkDescriptorImageInfo				   GetDescriptorImageInfoForAttachment(uint32_t attachmentIndex, VkSampler sampler) const;
		VkImage								   GetImageFromExternalIndex(uint32_t externalIndex);
		const std::vector<Attachment>&		   GetAttachments() const { return m_Attachments; }
		const std::vector<ExternalAttachment>& GetExternalAttachments() const { return m_ExternalAttachments; }

		VkFormat GetExternalColorImageFormat() const { return m_ExternalColorImageFormat; }
		VkFormat GetExternalDepthImageFormat() const { return m_ExternalDepthImageFormat; }

	private:
		void CreateFramebuffer();

	private:
		VulkanDevice& m_DeviceRef;

		VkFramebuffer					m_Framebuffer{};
		VkRenderPass					m_RenderPass{};
		uint32_t						m_Width, m_Height;
		std::vector<Attachment>			m_Attachments;
		std::vector<Subpass>			m_Subpasses;
		std::vector<SubpassDependency>	m_Dependencies;
		std::vector<ExternalAttachment> m_ExternalAttachments;
		VkFormat						m_ExternalColorImageFormat;
		VkFormat						m_ExternalDepthImageFormat;
	};
}

