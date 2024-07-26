#include "askygg/platform/windows/GLFWWindow.h"

#include "askygg/core/Window.h"
#include "askygg/core/Log.h"
#include "askygg/event/MouseEvent.h"
#include "askygg/event/KeyEvent.h"
#include "askygg/event/WindowEvent.h"
#include "askygg/core/Assert.h"

#include <glad/glad.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace askygg
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		YGG_LOG_CRITICAL("GLFW Error: {}", description);
	}

	GLFWWindow::GLFWWindow(const WindowProperties& properties)
	{
		Initialize(properties);
	}

	GLFWWindow::~GLFWWindow()
	{
		Shutdown();
	}

	void GLFWWindow::Initialize(const WindowProperties& properties)
	{
		YGG_ASSERT(glfwInit(), "Failed to initialize GLFW");

		m_Data.Title = properties.Title;
		m_Data.Width = properties.Width;
		m_Data.Height = properties.Height;

		glfwSetErrorCallback(GLFWErrorCallback);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		m_WindowHandle =
			glfwCreateWindow(m_Data.Width, m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);

		glfwMakeContextCurrent(m_WindowHandle);
		SetVSync(m_Data.VSync);

		m_Context = GraphicsContext::Create(m_WindowHandle);
		m_Context->Initialize();

		glfwSetWindowUserPointer(m_WindowHandle, static_cast<void*>(&m_Data));

		glfwSetWindowCloseCallback(m_WindowHandle, [](GLFWwindow* window) {
			WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

			WindowClosedEvent windowClosedEvent;
			data.Callback(windowClosedEvent);
		});

		glfwSetWindowSizeCallback(m_WindowHandle, [](GLFWwindow* window, int width, int height) {
			WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));
			data.Width = width;
			data.Height = height;

			WindowResizedEvent windowResizedEvent(width, height);
			data.Callback(windowResizedEvent);
		});

		glfwSetWindowPosCallback(m_WindowHandle, [](GLFWwindow* window, int xPosition, int yPosition) {
			WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

			WindowMovedEvent windowMovedEvent(xPosition, yPosition);
			data.Callback(windowMovedEvent);
		});

		glfwSetKeyCallback(
			m_WindowHandle, [](GLFWwindow* window, int keyCode, int scanCode, int action, int mods) {
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent keyPressedEvent(keyCode, 0);
						data.Callback(keyPressedEvent);
						break;
					}
					case GLFW_REPEAT:
					{
						KeyPressedEvent keyPressedEvent(keyCode, 1);
						data.Callback(keyPressedEvent);
						break;
					}
					case GLFW_RELEASE:
					{
						KeyReleasedEvent keyReleasedEvent(keyCode);
						data.Callback(keyReleasedEvent);
						break;
					}
				}
			});

		glfwSetMouseButtonCallback(
			m_WindowHandle, [](GLFWwindow* window, int button, int action, int mods) {
				WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent mouseButtonPressedEvent(button);
						data.Callback(mouseButtonPressedEvent);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent mouseButtonReleasedEvent(button);
						data.Callback(mouseButtonReleasedEvent);
						break;
					}
				}
			});

		glfwSetScrollCallback(m_WindowHandle, [](GLFWwindow* window, double xOffset, double yOffset) {
			WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

			MouseScrolledEvent mouseScrolledEvent(static_cast<float>(xOffset), static_cast<float>(yOffset));
			data.Callback(mouseScrolledEvent);
		});

		glfwSetCursorPosCallback(m_WindowHandle, [](GLFWwindow* window, double xPosition, double yPosition) {
			WindowData& data = *(static_cast<WindowData*>(glfwGetWindowUserPointer(window)));

			MouseMovedEvent mouseMovedEvent(static_cast<float>(xPosition), static_cast<float>(yPosition));
			data.Callback(mouseMovedEvent);
		});
	}

	void GLFWWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

	void GLFWWindow::ToggleIsMaximized(bool maximize) const
	{
		if (maximize)
			glfwMaximizeWindow(m_WindowHandle);
		else
			glfwRestoreWindow(m_WindowHandle);
	}

	void GLFWWindow::SetVSync(bool enable)
	{
		glfwSwapInterval(enable ? 1 : 0);
		m_Data.VSync = enable;
	}

	void GLFWWindow::Shutdown()
	{
		glfwDestroyWindow(m_WindowHandle);
		glfwTerminate();
	}

	bool GLFWWindow::IsVSync() const
	{
		return m_Data.VSync;
	}

	void GLFWWindow::ToggleIsHidden(bool hidden)
	{
		if (hidden)
			glfwHideWindow(m_WindowHandle);
		else
			glfwShowWindow(m_WindowHandle);
	}
} // namespace askygg
