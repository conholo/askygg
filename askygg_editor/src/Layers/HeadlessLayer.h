#pragma once

#include "askygg.h"

class HeadlessLayer : public askygg::Layer
{
public:
	HeadlessLayer(std::string inputDirectory, std::string outputDirectory,
		std::string configFilePath);
	void OnAttach() override;
	void OnDetach() override {}

private:
	std::string m_InputDirectory;
	std::string m_OutputDirectory;
	std::string m_ConfigFilePath;
};