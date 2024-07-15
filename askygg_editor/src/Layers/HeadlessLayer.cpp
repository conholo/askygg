#include "HeadlessLayer.h"
#include "ImageEditor.h"

HeadlessLayer::HeadlessLayer(std::string inputDirectory, std::string outputDirectory,
	std::string configFilePath)
	: m_InputDirectory(std::move(inputDirectory)), m_OutputDirectory(std::move(outputDirectory)), m_ConfigFilePath(std::move(configFilePath)) {}

void HeadlessLayer::OnAttach()
{
	askygg::Application::GetWindow().ToggleIsHidden(true);
	ImageEditor::InitializeImageEditor(m_InputDirectory, m_OutputDirectory, m_ConfigFilePath);
	ImageEditor::HeadlessProcessDirectory();
	ImageEditor::ShutdownImageEditor();
	askygg::Application::Close();
}
