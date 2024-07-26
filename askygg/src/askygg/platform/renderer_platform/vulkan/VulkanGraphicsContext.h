#pragma once

#include "askygg/renderer/GraphicsContext.h"
#include "VulkanDevice.h"

class GLFWwindow;

namespace askygg
{
	class VulkanGraphicsContext : public GraphicsContext
	{
	public:
		explicit VulkanGraphicsContext(GLFWwindow* windowHandle);

		VulkanDevice& GetDevice() const { return *m_Device; }

		void Initialize() override;
		void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
		VulkanDevice* m_Device;
	};
} // namespace askygg