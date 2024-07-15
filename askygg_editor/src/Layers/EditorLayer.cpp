#include "EditorLayer.h"
#include "ImageEditor.h"

EditorLayer::EditorLayer(std::string inputDirectory, std::string outputDirectory,
	std::string configFilePath)
	: m_InputDirectory(std::move(inputDirectory)), m_OutputDirectory(std::move(outputDirectory)), m_ConfigFilePath(std::move(configFilePath)) {}

void EditorLayer::OnAttach()
{
	ImageEditor::InitializeImageEditor(m_InputDirectory, m_OutputDirectory, m_ConfigFilePath);
	ImageEditor::LoadTextureSet(m_InputDirectory);
	m_Viewport = askygg::CreateRef<askygg::UI::Viewport>(ImageEditor::GetOutputFBO());
}

void EditorLayer::OnDetach() {}

void EditorLayer::OnUpdate(float deltaTime)
{
	ImageEditor::DrawActiveTexture();
}

void EditorLayer::OnImGuiRender()
{
	askygg::UI::Dockspace::Begin();
	askygg::UI::Dockspace::Draw();
	askygg::UI::StatisticsPanel::Draw();
	ImageEditor::DrawImageEditorUI();
	m_Viewport->Draw();
	askygg::UI::Dockspace::End();
}

void EditorLayer::OnEvent(askygg::Event& e)
{
	askygg::Camera::OnEvent(e);
}
