#pragma once
#include "askygg/renderer/PlatformRenderAPI.h"
#include "askygg/core/Window.h"
#include "VulkanDevice.h"
#include "VulkanFramebuffer.h"
#include "VulkanSwapchain.h"
#include "VulkanDescriptors.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanBuffer.h"
#include "askygg/renderer/Camera.h"
#include <memory>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>

namespace askygg
{
	class VulkanRenderer : public PlatformRenderAPI
	{
	public:
		explicit VulkanRenderer(Window& windowRef, VulkanDevice& deviceRef);
		~VulkanRenderer();
		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer& operator=(const VulkanRenderer&) = delete;

		void Draw(Camera& cameraRef);

		float GetAspectRatio() const { return m_Swapchain->GetExtentAspectRatio(); }

	public:
		void Initialize() override;
		void Shutdown() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear(bool colorBufferBit, bool depthBufferBit) override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;

	private:
		void CreatePipelines(VkDescriptorSetLayout globalSetLayout);
		void CreateDescriptorPools();
		void Render(FrameInfo& frameInfo, VkCommandBuffer drawCommandBuffer);

		void BeginOffscreenRenderPass(VkCommandBuffer offscreenCommandBuffer);
		void RenderSceneOffscreen(VkCommandBuffer offscreenCommandBufferm, FrameInfo& frameInfo);
		void EndOffscreenRenderPass(VkCommandBuffer offscreenCommandBuffer);

		VkRenderPass	GetSwapchainRenderPass() const { return m_Swapchain->GetRenderPass(); }
		bool		   	IsFrameInProgress() const { return m_IsFrameStarted; }

		VkCommandBuffer GetCurrentCommandBuffer() const
		{
			assert(m_IsFrameStarted && "Cannot get command buffer when frame is not in progress.");
			return m_DrawCommandBuffers[m_CurrentFrameIndex];
		}
		int GetFrameIndex() const
		{
			assert(m_IsFrameStarted && "Cannot get frame index when frame not in progress.");
			return m_CurrentFrameIndex;
		}

		VkCommandBuffer BeginFrame();
		void			EndFrame();

		void BeginSwapchainRenderPass(VkCommandBuffer commandBuffer);
		void RenderComposition(VkCommandBuffer commandBuffer, FrameInfo& frameInfo);
		void EndSwapchainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void RecreateSwapchain();

		void AllocateCommandBuffers();
		void CreateOffscreenFramebuffer();
		void CreateSynchronizationPrimitives();
		void CreateDescriptors();

		void CreateOffscreenGraphicsPipeline();
		void CreateOffscreenGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout);

		void CreateCompositionGraphicsPipeline();
		void CreateCompositionGraphicsPipelineLayout(VkDescriptorSetLayout globalSetLayout);

		void FreeCommandBuffers();

	private:
		Window&		  m_WindowRef;
		VulkanDevice& m_DeviceRef;

		std::unique_ptr<VulkanSwapchain> m_Swapchain;
		std::vector<VkCommandBuffer>	 m_DrawCommandBuffers;

		uint32_t m_CurrentImageIndex{ 0 };
		int		 m_CurrentFrameIndex{ 0 };
		bool	 m_IsFrameStarted{ false };

		std::unique_ptr<VulkanGraphicsPipeline> m_OffscreenGraphicsPipeline;
		VkPipelineLayout						m_OffscreenGraphicsPipelineLayout{};

		std::unique_ptr<VulkanGraphicsPipeline> m_CompositionGraphicsPipeline;
		VkPipelineLayout						m_CompositionGraphicsPipelineLayout{};

		VkSampler	m_OffscreenColorSampler{};
		VkSemaphore m_OffscreenSemaphore{};

		std::vector<VkCommandBuffer>	   m_OffscreenCommandBuffers;
		std::unique_ptr<VulkanFramebuffer> m_OffscreenFramebuffer;

		std::vector<std::unique_ptr<VulkanDescriptorPool>> m_OffscreenDescriptorPools;
		std::unique_ptr<VulkanDescriptorSetLayout>		   m_OffscreenDescriptorSetLayout;

		std::unique_ptr<VulkanDescriptorPool>	   m_CompositeDescriptorPool;
		std::unique_ptr<VulkanDescriptorSetLayout> m_CompositeDescriptorSetLayout;
	};
};
