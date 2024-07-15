#pragma once

#include "askygg/core/Window.h"
#include "askygg/renderer/GraphicsContext.h"

struct GLFWWindow;
namespace askygg
{
	class GLFWXPlatformWindow : public Window
	{
	public:
		explicit GLFWXPlatformWindow(const WindowProperties& props);
		~GLFWXPlatformWindow() override;
		void OnUpdate() override;

		void SetEventCallback(const EventCallbackFn& callback) override { m_Data.Callback = callback; }
		void ToggleIsHidden(bool hidden) override;

		void					   ToggleIsMaximized(bool maximize) const override;
		void					   SetVSync(bool enabled) override;
		[[nodiscard]] bool		   IsVSync() const override;
		[[nodiscard]] void*		   GetNativeWindow() const override { return m_WindowHandle; }
		[[nodiscard]] unsigned int GetWidth() const override { return m_Data.Width; }
		[[nodiscard]] unsigned int GetHeight() const override { return m_Data.Height; }

	private:
		virtual void Initialize(const WindowProperties& props);
		virtual void Shutdown();

	private:
		GLFWwindow*			 m_WindowHandle{};
		Ref<GraphicsContext> m_Context;

		struct WindowData
		{
			std::string	 Title;
			unsigned int Width, Height;
			bool		 VSync;

			EventCallbackFn Callback;
		};

		WindowData m_Data;
	};
} // namespace askygg