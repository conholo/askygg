#pragma once

#include "askygg.h"

class EditorLayer : public askygg::Layer
{
public:
	EditorLayer(std::string inputDirectory, std::string outputDirectory, std::string configFilePath);
	~EditorLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnImGuiRender() override;
	void OnEvent(askygg::Event& e) override;

private:
	askygg::Ref<askygg::UI::Viewport> m_Viewport;
	std::string						  m_InputDirectory;
	std::string						  m_OutputDirectory;
	std::string						  m_ConfigFilePath;
};