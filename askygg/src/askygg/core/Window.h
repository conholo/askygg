#pragma once

#include <functional>
#include <glm/glm.hpp>
#include <utility>
#include "askygg/event/Event.h"
#include "askygg/core/Memory.h"

struct GLFWwindow;

namespace askygg
{
	struct WindowProperties
	{
		std::string Title;
		uint32_t	Width;
		uint32_t	Height;

		explicit WindowProperties(std::string title = "askygg", uint32_t width = 1920,
			uint32_t height = 1080)
			: Title(std::move(title)), Width(width), Height(height) {}
	};

	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;
		virtual void OnUpdate() = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void ToggleIsMaximized(bool maximize) const = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual void ToggleIsHidden(bool hidden) = 0;

		[[nodiscard]] virtual bool	   IsVSync() const = 0;
		[[nodiscard]] virtual void*	   GetNativeWindow() const = 0;
		[[nodiscard]] virtual uint32_t GetWidth() const = 0;
		[[nodiscard]] virtual uint32_t GetHeight() const = 0;

		static Ref<Window> Create(const WindowProperties& props = WindowProperties());
	};
} // namespace askygg
