﻿
#include "askygg/ui/Dockspace.h"
#include <imgui.h>

namespace askygg::UI
{
	ImGuiDockNodeFlags Dockspace::s_DockspaceFlags;
	float			   Dockspace::s_GlobalMinWindowWidth = 370.0f;

	void Dockspace::Begin()
	{
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		const bool	opt_fullscreen = opt_fullscreen_persistant;
		s_DockspaceFlags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and
		// handle the pass-thru hole, so we ask Begin() to not render a background.
		if (s_DockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking,
		// otherwise any change of dockspace/settings would lead to windows being stuck in limbo and never
		// being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("askygg", &dockspaceOpen, window_flags);

		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);
	}

	void Dockspace::Draw()
	{
		ImGuiIO&	io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float		minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = s_GlobalMinWindowWidth;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("EngineDockspace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), s_DockspaceFlags);
		}
	}

	void Dockspace::End()
	{
		ImGui::End();
	}
} // namespace askygg::UI