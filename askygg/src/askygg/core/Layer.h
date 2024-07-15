#pragma once

#include "askygg/event/Event.h"

namespace askygg
{
	class Layer
	{
	public:
		explicit Layer(std::string name = "Layer")
			: m_Name(std::move(name)) {}
		virtual ~Layer() = default;

		virtual void OnAttach() = 0;
		virtual void OnDetach() = 0;
		virtual void OnUpdate(float deltaTime) {}
		virtual void OnEvent(Event& event) {}
		virtual void OnImGuiRender() {}

		[[nodiscard]] const std::string& GetName() const { return m_Name; }

	private:
		std::string m_Name;
	};
} // namespace askygg