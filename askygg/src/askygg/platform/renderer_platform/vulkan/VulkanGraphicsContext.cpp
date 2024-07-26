#include "VulkanGraphicsContext.h"
#include "askygg/core/Assert.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace askygg
{
	VulkanGraphicsContext::VulkanGraphicsContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		YGG_ASSERT(windowHandle, "Window handle is null!")
	}

	void VulkanGraphicsContext::Initialize()
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		m_Device = new VulkanDevice(m_WindowHandle);
	}

	void VulkanGraphicsContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
} // namespace askygg