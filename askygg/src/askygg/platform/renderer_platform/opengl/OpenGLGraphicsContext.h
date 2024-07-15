#pragma once

#include "askygg/renderer/GraphicsContext.h"

class GLFWwindow;
namespace askygg
{
	class OpenGLGraphicsContext : public GraphicsContext
	{
	public:
		explicit OpenGLGraphicsContext(GLFWwindow* windowHandle);

		void Initialize() override;
		void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
	};
} // namespace askygg