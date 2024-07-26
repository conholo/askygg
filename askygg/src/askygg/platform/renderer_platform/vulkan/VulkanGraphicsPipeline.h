#pragma once

#include <vulkan/vulkan.h>
#include "VulkanDevice.h"

namespace askygg
{
	class VulkanGraphicsPipeline
	{
	public:
		struct PipelineConfigInfo
		{
			PipelineConfigInfo() = default;
			PipelineConfigInfo(const PipelineConfigInfo&) = delete;
			PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

			std::vector<VkVertexInputBindingDescription>   BindingDescriptions{};
			std::vector<VkVertexInputAttributeDescription> AttributeDescriptions{};

			VkPipelineViewportStateCreateInfo	   ViewportInfo{};
			VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo{};
			VkPipelineRasterizationStateCreateInfo RasterizationInfo{};
			VkPipelineMultisampleStateCreateInfo   MultisampleInfo{};
			VkPipelineColorBlendAttachmentState	   ColorBlendAttachment{};
			VkPipelineColorBlendStateCreateInfo	   ColorBlendInfo{};
			VkPipelineDepthStencilStateCreateInfo  DepthStencilInfo{};

			std::vector<VkDynamicState>		 DynamicStateEnables;
			VkPipelineDynamicStateCreateInfo DynamicStateInfo{};

			VkPipelineLayout PipelineLayout = nullptr;
			VkRenderPass	 RenderPass = nullptr;
			uint32_t		 Subpass = 0;
			bool			 EmptyVertexInputState = false;
		};

		explicit VulkanGraphicsPipeline(VulkanDevice& deviceRef,
			const std::string&						  vertFilepath,
			const std::string&						  fragFilepath,
			const PipelineConfigInfo&				  configInfo);

		~VulkanGraphicsPipeline();

		VulkanGraphicsPipeline(const VulkanGraphicsPipeline&) = delete;
		VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;

		void		Bind(VkCommandBuffer commandBuffer);
		static void GetDefaultPipelineConfigInfo(PipelineConfigInfo& outInfo);
		static void EnableAlphaBlending(PipelineConfigInfo& outInfo);

	private:
		void CreateGraphicsPipeline(const std::string& vertFilepath,
			const std::string&						   fragFilepath,
			const PipelineConfigInfo&				   configInfo);

		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

	private:
		VulkanDevice&  m_DeviceRef;
		VkPipeline	   m_GraphicsPipeline;
		VkShaderModule m_VertShaderModule;
		VkShaderModule m_FragShaderModule;
	};
}

