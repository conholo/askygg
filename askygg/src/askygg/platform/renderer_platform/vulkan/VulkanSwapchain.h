#pragma once

#include "VulkanDevice.h"
#include <memory>

namespace askygg
{
	class VulkanSwapchain
	{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		VulkanSwapchain(VulkanDevice& device, VkExtent2D windowExtent);
		VulkanSwapchain(VulkanDevice& device, VkExtent2D windowExtent, std::shared_ptr<VulkanSwapchain> previous);
		~VulkanSwapchain();

		VulkanSwapchain(const VulkanSwapchain&) = delete;
		VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

		VkFramebuffer GetFrameBuffer(uint32_t index) { return m_SwapchainFramebuffers[index]; }
		VkRenderPass  GetRenderPass() { return m_RenderPass; }

		VkFormat GetSwapchainImageFormat() { return m_SwapchainImageFormat; }
		VkFormat GetSwapchainDepthFormat() { return m_SwapchainDepthFormat; }

		VkImageView GetImageView(int index) { return m_SwapchainImageViews[index]; }
		VkImage		GetImage(int index) { return m_SwapchainImages[index]; }
		size_t		GetImageCount() { return m_SwapchainImages.size(); }

		VkImageView	   GetDepthImageView(int index) { return m_DepthImageViews[index]; }
		VkImage		   GetDepthImage(int index) { return m_DepthImages[index]; }
		VkDeviceMemory GetDepthImageMemory(int index) { return m_DepthImageMemories[index]; }

		VkExtent2D			   GetSwapchainExtent() { return m_SwapchainExtent; }
		[[nodiscard]] uint32_t GetWidth() const { return m_SwapchainExtent.width; }
		[[nodiscard]] uint32_t GetHeight() const { return m_SwapchainExtent.height; }

		[[nodiscard]] float GetExtentAspectRatio() const
		{
			return static_cast<float>(m_SwapchainExtent.width) / static_cast<float>(m_SwapchainExtent.height);
		}

		[[nodiscard]] bool CompareSwapchainFormats(const VulkanSwapchain& swapchain) const
		{
			return swapchain.m_SwapchainDepthFormat == m_SwapchainDepthFormat && swapchain.m_SwapchainImageFormat == m_SwapchainImageFormat;
		}

		VkResult AcquireNextImage(uint32_t* imageIndex, VkSemaphore presentCompleteSemaphore);
		VkResult Present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);
		VkFormat FindDepthFormat();

	private:
		void Initialize();

		void CreateSwapchain();
		void CreateImageViews();
		void CreateDepthResources();
		void CreateRenderPass();
		void CreateFramebuffers();

		// Helper functions
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR   ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D		   ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat   m_SwapchainImageFormat{};
		VkFormat   m_SwapchainDepthFormat{};
		VkExtent2D m_SwapchainExtent{};

		std::vector<VkFramebuffer> m_SwapchainFramebuffers;
		VkRenderPass			   m_RenderPass{};

		std::vector<VkImage>		m_DepthImages;
		std::vector<VkDeviceMemory> m_DepthImageMemories;
		std::vector<VkImageView>	m_DepthImageViews;

		std::vector<VkImage>	 m_SwapchainImages;
		std::vector<VkImageView> m_SwapchainImageViews;

		VulkanDevice& m_DeviceRef;
		VkExtent2D	  m_WindowExtent;

		VkSwapchainKHR					 m_Swapchain{};
		std::shared_ptr<VulkanSwapchain> m_PreviousSwapchain;
	};
}
