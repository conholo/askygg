#pragma once

#include "askygg/core/Buffer.h"
#include "VulkanBuffer.h"
#include "askygg/renderer/Shader.h"
#include <vulkan/vulkan.h>

namespace askygg
{
	class VulkanComputePipeline
	{
	public:
		explicit VulkanComputePipeline(VulkanDevice& deviceRef);

		void CreatePipeline();

		void Begin(VkCommandBuffer cmdBuffer);
		void Dispatch(VkCommandBuffer computeCmdBuffer, const glm::uvec3& workGroups) const;
		void End(VkCommandBuffer computeCmdBuffer);

		void InsertBufferMemoryBarrier(
			VkCommandBuffer			 cmdBuffer,
			const Ref<VulkanBuffer>& buffer,
			VkPipelineStageFlagBits	 fromStage,
			VkAccessFlags			 fromAccess,
			VkPipelineStageFlagBits	 toStage,
			VkAccessFlags			 toAccess
		);

		void SetPushConstants(VkCommandBuffer computeCmdBuffer, Buffer constants) const;

		Ref<Shader> GetShader() { }

	private:

		VulkanDevice& m_DeviceRef;
	};
}