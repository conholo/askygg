#include "VulkanComputePipeline.h"
#include "VulkanUtils.h"

namespace askygg
{
	VulkanComputePipeline::VulkanComputePipeline(VulkanDevice& deviceRef)
		:m_DeviceRef(deviceRef)
	{
	}

	void VulkanComputePipeline::CreatePipeline()
	{
		VkDevice device = m_DeviceRef.GetDevice();
		auto descriptorSetLayouts = m_Shader->GetDescriptorSetLayouts();

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

		const auto& pushConstantRanges = m_Shader->GetPushConstantRanges();

		pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)pushConstantRanges.size();
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

		VK_CHECK_RESULT(vkCreatePipelineLayout(m_DeviceRef, &pipelineLayoutCreateInfo, nullptr, &m_ComputePipelineLayout));

		VkComputePipelineCreateInfo computePipelineCreateInfo{};
		computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.layout = m_ComputePipelineLayout;
		computePipelineCreateInfo.flags = 0;
		const auto& shaderStages = m_Shader->GetPipelineShaderStageCreateInfos();
		computePipelineCreateInfo.stage = shaderStages[0];

		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

		VK_CHECK_RESULT(vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));
		VK_CHECK_RESULT(vkCreateComputePipelines(
			device,
			m_PipelineCache,
			1,
			&computePipelineCreateInfo,
			nullptr,
			&m_ComputePipeline));

		SetDebugUtilsObjectName(device, VK_OBJECT_TYPE_PIPELINE, m_Shader->GetName(), m_ComputePipeline);
	}

	void VulkanComputePipeline::Begin(VkCommandBuffer computeCmdBuffer)
	{
		VkCommandBufferBeginInfo cmdBufferBeginInfo{};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		VK_CHECK_RESULT(vkBeginCommandBuffer(computeCmdBuffer, &cmdBufferBeginInfo));
	}

	void VulkanComputePipeline::Dispatch(VkCommandBuffer computeCmdBuffer, const glm::uvec3& workGroups) const
	{
		vkCmdDispatch(computeCmdBuffer, workGroups.x, workGroups.y, workGroups.z);
	}

	void VulkanComputePipeline::End(VkCommandBuffer computeCmdBuffer)
	{
		vkEndCommandBuffer(computeCmdBuffer);
	}

	void VulkanComputePipeline::InsertBufferMemoryBarrier(
		VkCommandBuffer cmdBuffer,
		const Ref<VulkanBuffer>& buffer,
		VkPipelineStageFlagBits fromStage,
		VkAccessFlags fromAccess,
		VkPipelineStageFlagBits toStage,
		VkAccessFlags toAccess)
	{
		VkBufferMemoryBarrier bufferMemoryBarrier = {};
		bufferMemoryBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		bufferMemoryBarrier.buffer = buffer->GetBuffer();
		bufferMemoryBarrier.offset = 0;
		bufferMemoryBarrier.size = VK_WHOLE_SIZE;
		bufferMemoryBarrier.srcAccessMask = (VkAccessFlags)fromAccess;
		bufferMemoryBarrier.dstAccessMask = (VkAccessFlags)toAccess;
		vkCmdPipelineBarrier(
			cmdBuffer,
			fromStage,
			toStage,
			0,
			0, nullptr,
			1, &bufferMemoryBarrier,
			0, nullptr);
	}

	void VulkanComputePipeline::SetPushConstants(VkCommandBuffer computeCmdBuffer, Buffer constants) const
	{
		vkCmdPushConstants(
			computeCmdBsuffer,
			m_ComputePipelineLayout,
			VK_SHADER_STAGE_COMPUTE_BIT,
			0,
			constants.Size,
			constants.Data);
	}

}