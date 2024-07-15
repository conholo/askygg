
#include "askygg/ui/Viewport.h"
#include "askygg/core/Application.h"

#include <imgui.h>

namespace askygg::UI
{
	Viewport::Viewport(const Ref<Framebuffer>& framebuffer)
		: m_Framebuffer(framebuffer) {}

	Viewport::Viewport() {}

	void Viewport::Draw()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");
		CalculateViewportSize();

        ImVec2 availContentSize = ImGui::GetContentRegionAvail(); // Available area in the current window
        ImVec2 cursorPos; // Cursor position to center the image
        cursorPos.x = (availContentSize.x - m_ViewportSize.x) * 0.5f;
        cursorPos.y = (availContentSize.y - m_ViewportSize.y) * 0.5f;

        // Get the ImDrawList instance to draw primitive shapes
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Draw a black rectangle filling the available space
        ImVec2 rectMin = ImGui::GetCursorScreenPos(); // Current cursor position
        ImVec2 rectMax = ImVec2(rectMin.x + availContentSize.x, rectMin.y + availContentSize.y);
        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(0, 0, 0, 255));

        // Set the cursor to the calculated position for the image
        ImGui::SetCursorPos(cursorPos);

        uint32_t textureID = m_Framebuffer->GetColorAttachmentID();
        ImGui::Image(reinterpret_cast<void*>(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y },
                     ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		ImGui::End();
		ImGui::PopStyleVar();
	}

	void Viewport::CalculateViewportSize()
	{
		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBoundsMin = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBoundsMax = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::GetApplication().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        // Calculate aspect ratio of the original image
        float aspectRatio = static_cast<float>(m_Framebuffer->GetCurrentSize().x) / static_cast<float>(m_Framebuffer->GetCurrentSize().y);

        float viewportWidth = viewportPanelSize.x;
        float viewportHeight = viewportPanelSize.x / aspectRatio;

        if(viewportHeight > viewportPanelSize.y)
        {
            viewportHeight = viewportPanelSize.y;
            viewportWidth = viewportPanelSize.y * aspectRatio;
        }

        m_ViewportSize = { viewportWidth, viewportHeight };
	}
} // namespace askygg::UI
