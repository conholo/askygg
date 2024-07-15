#include "MacOSApplication.h"
#include "askygg/renderer/Camera.h"
#include "askygg/renderer/RenderCommand.h"
#include "askygg/renderer/Renderer.h"
#include "askygg/core/Random.h"

namespace askygg
{
	MacOSApplication::MacOSApplication() {}

	MacOSApplication::~MacOSApplication() {}

	void MacOSApplication::Start()
	{
		m_ApplicationPool = NS::AutoreleasePool::alloc()->init();
		m_Window = CreateRef<MetalWindow>(m_WindowProperties);
		m_Device = CreateRef<MetalDevice>();
		m_ViewDelegate = CreateRef<MetalViewDelegate>();
		m_AppDelegate = CreateRef<MetalApplicationDelegate>(m_Window, m_Device, m_ViewDelegate);
		Camera::Initialize();
		RenderCommand::Initialize();
		// Renderer::Initialize();
		Random::Initialize();

		NS::Application* sharedApplication = NS::Application::sharedApplication();
		sharedApplication->setDelegate(&(*m_AppDelegate));
		sharedApplication->run();
	}

	void MacOSApplication::Exit()
	{
		m_ApplicationPool->release();
	}

	void MacOSApplication::Tick(float deltaTime) {}

	void MacOSApplication::OnEvent(Event& e) {}

	void MacOSApplication::CreateWindow(const WindowProperties& properties)
	{
		// Will be created once we create the Metal Application Delegate instance.
		m_WindowProperties = properties;
	}
} // namespace askygg