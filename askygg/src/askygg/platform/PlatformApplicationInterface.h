#pragma once

#include "askygg/event/Event.h"
#include "askygg/core/Window.h"

namespace askygg
{
	class PlatformApplicationInterface
	{
	public:
		virtual void	Start() = 0;
		virtual void	Exit() = 0;
		virtual void	OnEvent(Event& e) = 0;
		virtual void	CreateWindow(const WindowProperties& properties);
		virtual Window& GetWindow() = 0;
	};
} // namespace askygg
