#include "askygg.h"
#include "askygg/core/EntryPoint.h"
#include "Layers/EditorLayer.h"
#include "Layers/HeadlessLayer.h"

class EditorApplication : public askygg::Application
{
public:
	explicit EditorApplication(const askygg::ApplicationSpecification& spec)
		: askygg::Application(spec)
	{
		enum class Mode
		{
			Undefined,
			Headless,
			Editor
		};

		Mode		mode = Mode::Undefined;
		std::string inputDirectory = std::string();
		std::string outputDirectory = std::string();
		std::string configFilePath = std::string();

		for (int i = 0; i < spec.CommandLineArgs.Count; i++)
		{
			if (std::string(spec.CommandLineArgs[i]) == "--headless")
				mode = Mode::Headless;
			else if (std::string(spec.CommandLineArgs[i]) == "--editor")
				mode = Mode::Editor;
			else if (std::string(spec.CommandLineArgs[i]) == "--input_dir" && i + 1 < spec.CommandLineArgs.Count)
				inputDirectory = std::string(spec.CommandLineArgs[i + 1]);
			else if (std::string(spec.CommandLineArgs[i]) == "--output_dir" && i + 1 < spec.CommandLineArgs.Count)
				outputDirectory = std::string(spec.CommandLineArgs[i + 1]);
			else if (std::string(spec.CommandLineArgs[i]) == "--config_file" && i + 1 < spec.CommandLineArgs.Count)
				configFilePath = std::string(spec.CommandLineArgs[i + 1]);
		}

		switch (mode)
		{
			case Mode::Headless:
				YGG_LOG_INFO("Running in headless mode!");
				PushLayer(new HeadlessLayer(inputDirectory, outputDirectory, configFilePath));
				break;
			case Mode::Editor:
				YGG_LOG_INFO("Running in editor mode!");
				PushLayer(new EditorLayer(inputDirectory, outputDirectory, configFilePath));
				break;
			default:
				YGG_ASSERT(false, "Please specify a valid mode! (--headless or --editor)");
		}
	}

	~EditorApplication() override = default;
};

askygg::Application* askygg::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification spec;
	spec.Name = "Askygg Image Editor";
	spec.CommandLineArgs = args;
	return new EditorApplication(spec);
}