
#include "askygg/ui/StatisticsPanel.h"
#include "askygg/renderer/Renderer.h"

#include <imgui.h>

namespace askygg::UI
{
	void StatisticsPanel::Draw()
	{
		ImGui::Begin("Statistics");

		const Renderer::Statistics RenderStats = Renderer::RendererStatistics;

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
			ImGui::GetIO().Framerate);
		ImGui::Text("Vertex Count: %lu", RenderStats.VertexCount);
		ImGui::Text("Triangle Count: %lu", RenderStats.TriangleCount);
		ImGui::End();
	}
} // namespace askygg::UI
