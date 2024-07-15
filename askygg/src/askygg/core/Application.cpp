#include "askygg/core/Application.h"

#include <utility>
#include "askygg/core/Utility.h"
#include "askygg/core/Time.h"
#include "askygg/renderer/RenderCommand.h"
#include "askygg/renderer/Shader.h"
#include "askygg/renderer/Renderer.h"

#include "askygg/platform/linux/LinuxApplication.h"
#include "Random.h"

#ifdef E_PLATFORM_MACOS
	#include "askygg/platform/mac_os/MacOSApplication.h"
#endif

namespace askygg
{
	Application*				  Application::s_Instance = nullptr;
	PlatformApplicationInterface* Application::s_ApplicationDelegate = nullptr;

	PlatformApplicationInterface* CreateDelegate()
	{
#ifdef E_PLATFORM_WINDOWS
		return new LinuxApplication;
#endif
#ifdef E_PLATFORM_MACOS
		return new MacOSApplication;
#endif
#ifdef E_PLATFORM_LINUX
		return new LinuxApplication;
#endif
		YGG_ASSERT(false, "Unsupported platform");
	}

	Application::Application(ApplicationSpecification spec)
		: m_Specification(std::move(spec))
	{
		YGG_ASSERT(!s_Instance, "An instance of Application already exists!");
		s_Instance = this;

		s_ApplicationDelegate = CreateDelegate();
		YGG_ASSERT(s_ApplicationDelegate, "Failed to create platform-specific application delegate.");
		s_ApplicationDelegate->Start(WindowProperties());
		s_ApplicationDelegate->GetWindow().SetEventCallback(BIND_FN(Application::OnEvent));

		m_LayerStack = new LayerStack;
		m_ImGuiLayer = new ImGuiLayer;
		PushLayer(m_ImGuiLayer);
		Random::Seed(42);
	}

	Application::~Application()
	{
		delete m_LayerStack;
	}

	void Application::Run()
	{
		while (m_IsRunning)
		{
			Time::Tick();
			float deltaTime = Time::DeltaTime();

			Camera::OnUpdate(deltaTime);
			for (auto* layer : *m_LayerStack)
				layer->OnUpdate(deltaTime);

			m_ImGuiLayer->Begin();
			for (Layer* layer : *m_LayerStack)
				layer->OnImGuiRender();
			m_ImGuiLayer->End();

			s_ApplicationDelegate->Tick(deltaTime);
		}
	}

	void Application::Close()
	{
		s_Instance->m_IsRunning = false;
		s_ApplicationDelegate->Exit();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		s_ApplicationDelegate->OnEvent(event);

		m_ImGuiLayer->OnEvent(event);
		Camera::OnEvent(event);

		dispatcher.Dispatch<WindowClosedEvent>(BIND_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizedEvent>(BIND_FN(Application::OnWindowResize));

		for (auto it = (*m_LayerStack).rbegin(); it != (*m_LayerStack).rend(); ++it)
		{
			if (event.Handled)
				break;
			(*it)->OnEvent(event);
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		(*m_LayerStack).PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PopLayer(Layer* layer)
	{
		(*m_LayerStack).PopLayer(layer);
	}

	bool Application::OnWindowClose(WindowClosedEvent& windowCloseEvent)
	{
		Close();
		return true;
	}

	bool Application::OnWindowResize(WindowResizedEvent& windowResizeEvent)
	{
		RenderCommand::SetViewport(windowResizeEvent.GetWidth(), windowResizeEvent.GetHeight());
		return false;
	}
} // namespace askygg
