#pragma once

#include <string>
#include <map>
#include <vulkan/vulkan.h>

namespace askygg
{
	class VulkanShader
	{
	public:
		VulkanShader(const std::string& path);
		~VulkanShader();

		void Release();

	private:

		std::map<VkShaderStageFlagBits, std::vector<uint32_t>> m_ShaderData;
	};
}