#include "OpenGLGraphicsContext.h"
#include "askygg/core/Assert.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

namespace askygg
{
	OpenGLGraphicsContext::OpenGLGraphicsContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		YGG_ASSERT(windowHandle, "Window handle is null!")
	}

	void OpenGLGraphicsContext::Initialize()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		YGG_ASSERT(status, "Failed to initialize Glad!");

		YGG_LOG_INFO("OpenGL Info:");
		YGG_LOG_INFO("  Vendor: {0}", glGetString(GL_VENDOR));
		YGG_LOG_INFO("  Renderer: {0}", glGetString(GL_RENDERER));
		YGG_LOG_INFO("  Version: {0}", glGetString(GL_VERSION));

		YGG_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5),
			"askygg requires at least OpenGL version 4.5!");
	}

	void OpenGLGraphicsContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
} // namespace askygg