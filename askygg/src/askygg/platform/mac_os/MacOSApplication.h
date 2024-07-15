#pragma once

#include "askygg/core/Application.h"
#include "askygg/platform/renderer_platform/metal/MetalApplicationDelegate.h"

namespace askygg
{
	class MacOSApplication : public PlatformApplicationInterface
	{
	public:
		MacOSApplication();
		~MacOSApplication();

		void Start() override;
		void Exit() override;
		void Tick(float deltaTime) override;
		void OnEvent(Event& e) override;

		MetalApplicationDelegate& GetMetalAppDelegate() { return *m_AppDelegate; }

		void	CreateWindow(const WindowProperties& properties) override;
		Window& GetWindow() override { return m_AppDelegate->GetWindow(); };

	private:
		WindowProperties m_WindowProperties;

		Ref<MetalWindow>	   m_Window;
		Ref<MetalDevice>	   m_Device;
		Ref<MetalViewDelegate> m_ViewDelegate;

		NS::AutoreleasePool*		  m_ApplicationPool = nullptr;
		Ref<MetalApplicationDelegate> m_AppDelegate = nullptr;
	};
} // namespace askygg