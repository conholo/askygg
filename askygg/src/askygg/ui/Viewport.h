#pragma once

#include "askygg/renderer/Framebuffer.h"
#include "askygg/core/Memory.h"
#include <glm/glm.hpp>

namespace askygg::UI
{
	class Viewport
	{
	public:
		Viewport();
		Viewport(const Ref<Framebuffer>& framebuffer);

		void SetFramebuffer(const Ref<Framebuffer>& framebuffer) { m_Framebuffer = framebuffer; }
		void Draw();

		const glm::vec2& GetViewportSize() const { return m_ViewportSize; }
		const glm::vec2& GetViewportBoundsMin() const { return m_ViewportBoundsMin; }
		const glm::vec2& GetViewportBoundsMax() const { return m_ViewportBoundsMax; }

		void CalculateViewportSize();

	private:
		Ref<Framebuffer> m_Framebuffer;
		glm::vec2		 m_ViewportSize{ 0.0f };
		glm::vec2		 m_ViewportBoundsMin{ 0.0f };
		glm::vec2		 m_ViewportBoundsMax{ 0.0f };
		bool			 m_ViewportFocused = false;
		bool			 m_ViewportHovered = false;
	};
} // namespace askygg::UI
