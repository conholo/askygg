#include "VulkanRenderer.h"
#include "VulkanUtils.h"
#include "GLFW/glfw3.h"

#include <memory>
#include <sstream>
#include <array>

namespace askygg
{
	struct CompositionPushConstant
	{
		int DebugDisplay;
	};

	VulkanRenderer::VulkanRenderer(Window& windowRef, VulkanDevice& deviceRef)
		: m_WindowRef(windowRef), m_DeviceRef(deviceRef)
	{
		RecreateSwapchain();

		AllocateCommandBuffers();
		CreateOffscreenFramebuffer();
		CreateDescriptors();
		CreateSynchronizationPrimitives();
	}

	void VulkanRenderer::CreateDescriptorPools()
	{
		std::vector<std::vector<VkDescriptorSet>> offscreenDescriptorSets(VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);

		m_OffscreenDescriptorPools.resize(VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);
		auto offscreenFramePoolBuilder = VulkanDescriptorPool::Builder(m_DeviceRef)
											 .SetMaxSets(VulkanSwapchain::MAX_FRAMES_IN_FLIGHT)
											 .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VulkanSwapchain::MAX_FRAMES_IN_FLIGHT * 2)
											 .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VulkanSwapchain::MAX_FRAMES_IN_FLIGHT)
											 .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

		for (auto& offscreenFramePool : m_OffscreenDescriptorPools)
			offscreenFramePool = offscreenFramePoolBuilder.Build();

		m_CompositeDescriptorPool = VulkanDescriptorPool::Builder(m_DeviceRef)
										.SetMaxSets(VulkanSwapchain::MAX_FRAMES_IN_FLIGHT)
										.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VulkanSwapchain::MAX_FRAMES_IN_FLIGHT * 3)
										.Build();
	}

	void VulkanRenderer::CreatePipelines()
	{
		CreateCompositionGraphicsPipelineLayout();
		CreateCompositionGraphicsPipeline();

		CreateOffscreenGraphicsPipelineLayout();
		CreateOffscreenGraphicsPipeline();
	}

	void VulkanRenderer::BeginOffscreenRenderPass(VkCommandBuffer offscreenCommandBuffer)
	{
		// Output of this pass is writing to all attachments for the offscreen FBO.
		// Clear values for all attachments written in the fragment shader

		// Begin recording the offscreen command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		std::array<VkClearValue, 4> clearValues{};
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[3].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = m_OffscreenFramebuffer->GetRenderPass();
		renderPassBeginInfo.framebuffer = m_OffscreenFramebuffer->GetFramebuffer();
		renderPassBeginInfo.renderArea.extent.width = m_OffscreenFramebuffer->GetWidth();
		renderPassBeginInfo.renderArea.extent.height = m_OffscreenFramebuffer->GetHeight();
		renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassBeginInfo.pClearValues = clearValues.data();

		VK_CHECK_RESULT(vkBeginCommandBuffer(offscreenCommandBuffer, &beginInfo));
		vkCmdBeginRenderPass(offscreenCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_OffscreenFramebuffer->GetWidth());
		viewport.height = static_cast<float>(m_OffscreenFramebuffer->GetHeight());
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ { 0, 0 }, renderPassBeginInfo.renderArea.extent };
		vkCmdSetViewport(offscreenCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(offscreenCommandBuffer, 0, 1, &scissor);
	}

	void VulkanRenderer::RenderSceneOffscreen(VkCommandBuffer offscreenCommandBuffer, FrameInfo& frameInfo)
	{
		m_OffscreenDescriptorPools[frameInfo.FrameIndex]->ResetPool();
		m_OffscreenGraphicsPipeline->Bind(offscreenCommandBuffer);

		vkCmdBindDescriptorSets(
			offscreenCommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_OffscreenGraphicsPipelineLayout,
			0,
			1,
			&m_GlobalDescriptorSet,
			0,
			nullptr);

		for (auto& entry : frameInfo.GameObjects)
		{
			auto& obj = entry.second;
			if (obj.ObjectModel == nullptr)
				continue;

			// Object-specific transform data
			auto gameObjectBufferInfo = obj.GetBufferInfo(frameInfo.FrameIndex);
			// Object-specific material data
			auto diffuseImageInfo = obj.DiffuseMap->GetImage()->GetDescriptorInfo();
			auto normalImageInfo = obj.NormalMap->GetImage()->GetDescriptorInfo();

			VkDescriptorSet gameObjectDescriptorSet;
			VulkanDescriptorWriter(*m_OffscreenDescriptorSetLayout, *m_OffscreenDescriptorPools[frameInfo.FrameIndex])
				.WriteBuffer(0, &gameObjectBufferInfo)
				.WriteImage(1, &diffuseImageInfo)
				.WriteImage(2, &normalImageInfo)
				.Build(gameObjectDescriptorSet);

			vkCmdBindDescriptorSets(
				offscreenCommandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_OffscreenGraphicsPipelineLayout,
				1, // starting set (0 is the globalDescriptorSet, 1 is the set specific to this system)
				1, // set count
				&gameObjectDescriptorSet,
				0,
				nullptr);

			obj.ObjectModel->Bind(offscreenCommandBuffer);
			obj.ObjectModel->Draw(offscreenCommandBuffer);
		}
	}

	void VulkanRenderer::EndOffscreenRenderPass(VkCommandBuffer offscreenCommandBuffer)
	{
		vkCmdEndRenderPass(offscreenCommandBuffer);
		VK_CHECK_RESULT(vkEndCommandBuffer(offscreenCommandBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &offscreenCommandBuffer;
		// Wait for swap chain presentation to finish
		VkSemaphore& currentImageAvailableSemaphore = m_Swapchain->GetCurrentImageAvailableSemaphore();
		submitInfo.pWaitSemaphores = &currentImageAvailableSemaphore;
		// Signal ready with offscreen semaphore
		submitInfo.pSignalSemaphores = &m_OffscreenSemaphore;

		vkQueueSubmit(m_DeviceRef.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_DeviceRef.GetGraphicsQueue());
	}

	void VulkanRenderer::Render(FrameInfo& frameInfo, VkCommandBuffer drawCommandBuffer)
	{
		VkCommandBuffer offscreenCommandBuffer = m_OffscreenCommandBuffers[frameInfo.FrameIndex];
		BeginOffscreenRenderPass(offscreenCommandBuffer);
		RenderSceneOffscreen(offscreenCommandBuffer, frameInfo);
		EndOffscreenRenderPass(offscreenCommandBuffer);

		BeginSwapchainRenderPass(drawCommandBuffer);
		RenderComposition(drawCommandBuffer, frameInfo);
		EndSwapchainRenderPass(drawCommandBuffer);
	}

	VulkanRenderer::~VulkanRenderer()
	{
		FreeCommandBuffers();
	}

	void VulkanRenderer::RecreateSwapchain()
	{
		auto extent = m_Swapchain->GetSwapchainExtent();
		while (extent.width == 0 || extent.height == 0)
		{
			extent = m_Swapchain->GetSwapchainExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(m_DeviceRef.GetDevice());

		if (m_Swapchain == nullptr)
		{
			m_Swapchain = std::make_unique<VulkanSwapchain>(m_DeviceRef, extent);
		}
		else
		{
			std::shared_ptr<VulkanSwapchain> oldSwapChain = std::move(m_Swapchain);
			m_Swapchain = std::make_unique<VulkanSwapchain>(m_DeviceRef, extent, oldSwapChain);

			if (!oldSwapChain->CompareSwapchainFormats(*m_Swapchain))
			{
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}

		if (m_OffscreenFramebuffer)
			m_OffscreenFramebuffer->Resize(m_Swapchain->GetWidth(), m_Swapchain->GetHeight());
	}

	VkCommandBuffer VulkanRenderer::BeginFrame()
	{
		assert(!m_IsFrameStarted && "Can't call BeginFrame while already in progress.");

		auto result = m_Swapchain->AcquireNextImage(&m_CurrentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		m_IsFrameStarted = true;

		auto					 commandBuffer = GetCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		return commandBuffer;
	}

	void VulkanRenderer::BeginSwapchainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_IsFrameStarted && "Can't call BeginSwapChainRenderPass if frame is not in progress");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_Swapchain->GetRenderPass();
		renderPassInfo.framebuffer = m_Swapchain->GetFrameBuffer(m_CurrentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_Swapchain->GetSwapchainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_Swapchain->GetSwapchainExtent().width);
		viewport.height = static_cast<float>(m_Swapchain->GetSwapchainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ { 0, 0 }, m_Swapchain->GetSwapchainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VulkanRenderer::RenderComposition(VkCommandBuffer commandBuffer, FrameInfo& frameInfo)
	{
		m_CompositeDescriptorPool->ResetPool();
		m_CompositionGraphicsPipeline->Bind(commandBuffer);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_CompositionGraphicsPipelineLayout,
			0,
			1,
			&frameInfo.GlobalDescriptorSet,
			0,
			nullptr);

		VkDescriptorImageInfo positionInfo = m_OffscreenFramebuffer->GetDescriptorImageInfoForAttachment(0, m_OffscreenColorSampler);
		VkDescriptorImageInfo normalInfo = m_OffscreenFramebuffer->GetDescriptorImageInfoForAttachment(1, m_OffscreenColorSampler);
		VkDescriptorImageInfo albedoInfo = m_OffscreenFramebuffer->GetDescriptorImageInfoForAttachment(2, m_OffscreenColorSampler);

		VkDescriptorSet compositeDescriptorSet;
		VulkanDescriptorWriter(*m_CompositeDescriptorSetLayout, *m_CompositeDescriptorPool)
			.WriteImage(0, &positionInfo)
			.WriteImage(1, &normalInfo)
			.WriteImage(2, &albedoInfo)
			.Build(compositeDescriptorSet);

		CompositionPushConstant push{};
		push.DebugDisplay = 0;
		vkCmdPushConstants(commandBuffer,
			m_CompositionGraphicsPipelineLayout,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(CompositionPushConstant),
			&push);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_CompositionGraphicsPipelineLayout,
			1,
			1,
			&compositeDescriptorSet,
			0,
			nullptr);

		// Final composition
		// This is done by simply drawing a full screen quad
		// The fragment shader then combines the deferred attachments into the final image
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
	}

	void VulkanRenderer::EndSwapchainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_IsFrameStarted && "Can't call EndSwapChainRenderPass if frame is not in progress.");
		assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
		vkCmdEndRenderPass(commandBuffer);
	}

	void VulkanRenderer::EndFrame()
	{
		assert(m_IsFrameStarted && "Can't call EndFrame while frame is not in progress.");

		auto commandBuffer = GetCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}

		auto result = m_Swapchain->SubmitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_WindowRef.WasWindowResized())
		{
			m_WindowRef.ResetWindowResizedFlag();
			RecreateSwapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swapchain image!");
		}

		m_IsFrameStarted = false;
		m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % VulkanSwapchain::MAX_FRAMES_IN_FLIGHT;
	}

	/*
 * Resources Allocation and Initialization Begin
	 */

	void VulkanRenderer::AllocateCommandBuffers()
	{
		m_OffscreenCommandBuffers.resize(VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo offscreenCmdAllocInfo{};
		offscreenCmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		offscreenCmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		offscreenCmdAllocInfo.commandPool = m_DeviceRef.GetGraphicsCommandPool();
		offscreenCmdAllocInfo.commandBufferCount = static_cast<uint32_t>(m_OffscreenCommandBuffers.size());

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_DeviceRef.GetDevice(), &offscreenCmdAllocInfo, m_OffscreenCommandBuffers.data()));

		m_DrawCommandBuffers.resize(VulkanSwapchain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo compositeAllocInfo{};
		compositeAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		compositeAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		compositeAllocInfo.commandPool = m_DeviceRef.GetGraphicsCommandPool();
		compositeAllocInfo.commandBufferCount = static_cast<uint32_t>(m_DrawCommandBuffers.size());

		if (vkAllocateCommandBuffers(m_DeviceRef.GetDevice(), &compositeAllocInfo, m_DrawCommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}
	}

	void VulkanRenderer::CreateSynchronizationPrimitives()
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo{};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreateSemaphore(m_DeviceRef.GetDevice(), &semaphoreCreateInfo, nullptr, &m_OffscreenSemaphore));
	}

	void VulkanRenderer::CreateOffscreenFramebuffer()
	{
		std::vector<VulkanFramebuffer::Attachment::Specification> specs{ 4 };

		VulkanFramebuffer::Attachment::Specification positionsAttachmentSpec = {
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		};
		VulkanFramebuffer::Attachment::Specification normalsAttachmentSpec = {
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		};
		VulkanFramebuffer::Attachment::Specification albedosAttachmentSpec = {
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
		};

		VkFormat validDepthFormat = m_Swapchain->GetSwapchainDepthFormat();
		assert(validDepthFormat);

		VulkanFramebuffer::Attachment::Specification depthAttachmentSpec = {
			validDepthFormat,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
		};

		std::vector<VulkanFramebuffer::Attachment::Specification> attachmentSpecs{
			positionsAttachmentSpec,
			normalsAttachmentSpec,
			albedosAttachmentSpec,
			depthAttachmentSpec
		};
		uint32_t swapChainWidth = m_Swapchain->GetWidth();
		uint32_t swapChainHeight = m_Swapchain->GetHeight();

		m_OffscreenFramebuffer = std::make_unique<VulkanFramebuffer>(
			m_DeviceRef,
			swapChainWidth, swapChainHeight,
			attachmentSpecs);

		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.maxAnisotropy = 1.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 1.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(m_DeviceRef.GetDevice(), &samplerCreateInfo, nullptr, &m_OffscreenColorSampler));
	}

	void VulkanRenderer::CreateOffscreenGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		const std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			globalSetLayout,
			m_OffscreenDescriptorSetLayout->GetDescriptorSetLayout()
		};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		if (vkCreatePipelineLayout(m_DeviceRef.GetDevice(), &pipelineLayoutInfo, nullptr,
				&m_OffscreenGraphicsPipelineLayout)
			!= VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void VulkanRenderer::CreateOffscreenGraphicsPipeline()
	{
		assert(m_OffscreenGraphicsPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

		VulkanGraphicsPipeline::PipelineConfigInfo pipelineConfig{};
		VulkanGraphicsPipeline::GetDefaultPipelineConfigInfo(pipelineConfig);

		// Blend attachment states required for all color attachments
		// This is important, as color write mask will otherwise be 0x0 which means
		// all components of the framebuffer are readonly.
		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
		pipelineColorBlendAttachmentState.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		pipelineColorBlendAttachmentState.blendEnable = VK_FALSE;
		blendAttachmentState = pipelineColorBlendAttachmentState;

		pipelineConfig.ColorBlendInfo.attachmentCount = 1;
		pipelineConfig.ColorBlendInfo.pAttachments = &blendAttachmentState;

		pipelineConfig.RenderPass = m_OffscreenFramebuffer->GetRenderPass();
		pipelineConfig.PipelineLayout = m_OffscreenGraphicsPipelineLayout;
		m_OffscreenGraphicsPipeline = std::make_unique<VulkanGraphicsPipeline>(
			m_DeviceRef,
			"../assets/shaders/march.vert.spv",
			"../assets/shaders/march.frag.spv",
			pipelineConfig);
	}

	void VulkanRenderer::CreateCompositionGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		const std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			globalSetLayout,
			m_CompositeDescriptorSetLayout->GetDescriptorSetLayout()
		};

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(CompositionPushConstant);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_DeviceRef.GetDevice(), &pipelineLayoutInfo, nullptr, &m_CompositionGraphicsPipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void VulkanRenderer::CreateCompositionGraphicsPipeline()
	{
		assert(m_CompositionGraphicsPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

		VulkanGraphicsPipeline::PipelineConfigInfo pipelineConfig{};
		VulkanGraphicsPipeline::GetDefaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.RenderPass = m_Swapchain->GetRenderPass();
		pipelineConfig.PipelineLayout = m_CompositionGraphicsPipelineLayout;
		pipelineConfig.EmptyVertexInputState = true;
		m_CompositionGraphicsPipeline = std::make_unique<VulkanGraphicsPipeline>(
			m_DeviceRef,
			"../assets/shaders/composition.vert.spv",
			"../assets/shaders/texture_display.frag.spv",
			pipelineConfig);
	}

	void VulkanRenderer::CreateDescriptors()
	{
		// Scene (offscreen)
		m_OffscreenDescriptorSetLayout =
			VulkanDescriptorSetLayout::Builder(m_DeviceRef)
				// Binding 0: GameObject UBO
				.AddBinding(
					0,
					VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
					VK_SHADER_STAGE_VERTEX_BIT)
				// Binding 1: Albedo Texture
				.AddBinding(
					1,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT)
				// Binding 2: Normal Texture
				.AddBinding(
					2,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT)
				.Build();

		// Composite (fullscreen)
		m_CompositeDescriptorSetLayout =
			VulkanDescriptorSetLayout::Builder(m_DeviceRef)
				// Binding 0: Position Sampler
				.AddBinding(
					0,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT)
				// Binding 1: Albedo Sampler
				.AddBinding(
					1,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT)
				// Binding 2: Normal Sampler
				.AddBinding(
					2,
					VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					VK_SHADER_STAGE_FRAGMENT_BIT)
				.Build();
	}

	/*
 * Resources Cleanup Begin
	 */

	void VulkanRenderer::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(
			m_DeviceRef.GetDevice(),
			m_DeviceRef.GetGraphicsCommandPool(),
			static_cast<uint32_t>(m_DrawCommandBuffers.size()),
			m_DrawCommandBuffers.data());
		m_DrawCommandBuffers.clear();
	}
}

