#pragma once

#include "askygg/scene/Scene.h"
#include "askygg/scene/Entity.h"
#include "askygg/renderer/RenderPass.h"
#include "askygg/renderer/Camera.h"
#include "askygg/event/Event.h"
#include "askygg/renderer/Texture.h"
#include "askygg/ui/Viewport.h"

#include "ImagePass.h"
#include "Passes/OutputComputePass.h"

#include <chrono>

class ImageEditor
{
public:
	static void InitializeImageEditor(const std::string& inputDirectory,
		const std::string&								 outputDirectory,
		const std::string&								 settingsFileName);
	static void ShutdownImageEditor();

	static void DrawActiveTexture();
	static void DrawImageEditorUI();

	static void HeadlessProcessDirectory();
	static void LoadTextureSet(const std::string& directoryPath);

	static const askygg::Ref<askygg::Framebuffer>& GetOutputFBO();

private:
	static void SubmitPipeline(const glm::vec2& targetSize, uint32_t targetTextureID, bool display = false, bool profile = true);
	static void SaveTexture(const askygg::Texture2D& texture, const std::string& outputDirectory, bool profile = true);

    static void SetBloomPass(BloomType bloomType);

	static void InitializeOutputDisplayPass();
	static void SubmitOutputDisplayPass(uint32_t displayTextureID);
    static void SavePassOrder();

private:
	static askygg::Ref<askygg::RenderPass> s_DisplayRenderPass;

	static std::string s_SettingsFileName;
	static std::string s_InputDirectory;
	static std::string s_OutputDirectory;

	static uint32_t				 s_ActiveTextureIndex;
	static std::vector<uint32_t> s_TextureSet;

	static std::unordered_map<ImagePassType, double>				 s_PassExecutionTime;
	static std::vector<std::pair<ImagePassType, double>>			 s_SortedExecutionTimes;
	static std::chrono::high_resolution_clock::time_point			 s_LastSortTime;

    static std::unordered_map<ImagePassType, askygg::Ref<ImagePass>> s_AllPasses;
    static std::vector<ImagePassType> s_OrderedPassTypes;
    static askygg::Ref<ImagePass> s_ActiveBloomPass;
    static BloomType s_ActiveBloomPassType;

    inline static bool s_DisplayUnprocessedInput = false;
    inline static glm::vec2 s_LastRecordedViewportSize{};
};
