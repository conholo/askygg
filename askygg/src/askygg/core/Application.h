#pragma once

#include "askygg/core/Window.h"
#include "askygg/event/Event.h"
#include "askygg/event/WindowEvent.h"
#include "askygg/core/LayerStack.h"
#include "askygg/core/Memory.h"
#include "askygg/imgui/ImGuiLayer.h"
#include "askygg/core/Assert.h"

namespace askygg
{
	struct ApplicationCommandLineArgs
	{
		int	   Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			YGG_ASSERT(index < Count, "Invalid command line arg.");
			return Args[index];
		}
	};

	struct ApplicationSpecification
	{
		std::string				   Name = "Askygg Image Editor";
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class PlatformApplicationInterface
	{
	public:
		virtual void	Start(const WindowProperties& properties) = 0;
		virtual void	Exit() = 0;
		virtual void	Tick(float deltaTime) = 0;
		virtual void	OnEvent(Event& e) = 0;
		virtual Window& GetWindow() = 0;
	};

	class Application
	{
	public:
		explicit Application(ApplicationSpecification spec);
		virtual ~Application();

		void		Run();
		static void Close();
		void		OnEvent(Event& event);

		static Application&					 GetApplication() { return *s_Instance; }
		static PlatformApplicationInterface* GetPlatformAppInterface() { return s_ApplicationDelegate; }
		static Window&						 GetWindow() { return s_ApplicationDelegate->GetWindow(); }
		[[nodiscard]] const std::string&	 GetName() const { return m_Specification.Name; }
		[[nodiscard]] ImGuiLayer*			 GetImGuiLayer() const { return m_ImGuiLayer; }

		void PushLayer(Layer* layer);
		void PopLayer(Layer* layer);

	private:
		bool OnWindowClose(WindowClosedEvent& windowCloseEvent);
		bool OnWindowResize(WindowResizedEvent& windowResizeEvent);

	private:
	private:
		ApplicationSpecification m_Specification;
		bool					 m_IsRunning = true;
		ImGuiLayer*				 m_ImGuiLayer;
		LayerStack*				 m_LayerStack;

		static Application*					 s_Instance;
		static PlatformApplicationInterface* s_ApplicationDelegate;
	};

	// To be defined in CLIENT
	Application* CreateApplication(ApplicationCommandLineArgs args);
} // namespace askygg