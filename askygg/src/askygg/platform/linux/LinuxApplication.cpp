#include "LinuxApplication.h"
#include "askygg/renderer/Camera.h"
#include "askygg/renderer/RenderCommand.h"
#include "askygg/renderer/Renderer.h"
#include "askygg/core/Random.h"
#include "askygg/renderer/PlatformRenderAPI.h"

namespace askygg
{
	LinuxApplication::LinuxApplication() {}

	LinuxApplication::~LinuxApplication() {}

	void LinuxApplication::Start(const WindowProperties& properties)
	{
		Camera::Initialize();
		PlatformRenderAPI::InitializePlatformRendererType();
		m_Window = Window::Create(properties);
		RenderCommand::Initialize();
		Renderer::Initialize();
		Random::Initialize();
	}

	void LinuxApplication::Exit()
	{
		Renderer::Shutdown();
	}

	void LinuxApplication::Tick(float deltaTime)
	{
		m_Window->OnUpdate();
	}

	void LinuxApplication::OnEvent(Event& e) {}
} // namespace askygg