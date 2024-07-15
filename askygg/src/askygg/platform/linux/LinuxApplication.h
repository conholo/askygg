#pragma once

#include "askygg/core/Application.h"

namespace askygg
{
	class LinuxApplication : public PlatformApplicationInterface
	{
	public:
		LinuxApplication();
		~LinuxApplication();

		void	Start(const WindowProperties& properties) override;
		void	Exit() override;
		void	Tick(float deltaTime) override;
		void	OnEvent(Event& e) override;
		Window& GetWindow() override { return *m_Window; };

	private:
		Ref<Window> m_Window;
	};
} // namespace askygg