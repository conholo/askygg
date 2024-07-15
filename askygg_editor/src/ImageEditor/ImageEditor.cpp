#include "ImageEditor.h"

#include "Passes/MultiPassBloomPass.h"
#include "Passes/BarrelDistortionPass.h"
#include "Passes/ChromaticAberrationPass.h"
#include "Passes/SobelPass.h"
#include "Passes/ContrastBrightnessPass.h"
#include "Passes/RadialBlurPass.h"
#include "Passes/SharpenPass.h"
#include "Passes/HSVAdjustmentPass.h"
#include "Passes/VignettePass.h"
#include "Passes/LinearizePass.h"

#include "askygg/renderer/Renderer.h"
#include "askygg/renderer/Framebuffer.h"
#include "askygg/renderer/Shader.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include "askygg/platform/renderer_platform/opengl/OpenGLTimer.h"

#include <imgui.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include "yaml-cpp/yaml.h"
#include "askygg/core/Timer.h"
#include "Passes/RadialBloomPass.h"
#include "imgui_internal.h"
#include <filesystem>

std::vector<ImagePassType>                                  ImageEditor::s_OrderedPassTypes;
std::unordered_map<ImagePassType, askygg::Ref<ImagePass>>   ImageEditor::s_AllPasses;
askygg::Ref<ImagePass>                                      ImageEditor::s_ActiveBloomPass;
BloomType                                                   ImageEditor::s_ActiveBloomPassType;
askygg::Ref<askygg::RenderPass>                             ImageEditor::s_DisplayRenderPass;

std::unordered_map<ImagePassType, double>                   ImageEditor::s_PassExecutionTime;
std::vector<std::pair<ImagePassType, double>>               ImageEditor::s_SortedExecutionTimes;
std::chrono::high_resolution_clock::time_point              ImageEditor::s_LastSortTime = std::chrono::high_resolution_clock::now();

std::string ImageEditor::s_SettingsFileName;
std::string ImageEditor::s_InputDirectory;
std::string ImageEditor::s_OutputDirectory;

uint32_t              ImageEditor::s_ActiveTextureIndex = 0;
std::vector<uint32_t> ImageEditor::s_TextureSet{};


std::vector<std::string> GetDefaultPassOrderToString()
{
    std::vector<ImagePassType> defaultPassOrder =
    {
            ImagePassType::ContrastBrightness,
            ImagePassType::HueShift,
            ImagePassType::Sobel,
            ImagePassType::RadialBlur,
            ImagePassType::ChromaticAberration,
            ImagePassType::BarrelDistortion,
            ImagePassType::Vignette,
            ImagePassType::Sharpen,
    };
    // Load active passes
    std::vector<std::string> defaultPassOrderStrings;
    for(auto & i : defaultPassOrder)
        defaultPassOrderStrings.push_back(ImagePass::ImagePassTypeToString(i));

    return defaultPassOrderStrings;
}

void ImageEditor::SetBloomPass(BloomType bloomType)
{
    switch(bloomType)
    {
        case BloomType::Radial:     s_ActiveBloomPass = s_AllPasses[ImagePassType::RadialBloom]; break;
        case BloomType::MultiPass:  s_ActiveBloomPass = s_AllPasses[ImagePassType::MultiPassBloom]; break;
        default: s_ActiveBloomPass = nullptr; break;
    }

    // Notify the output shader of the change.
    auto outputPass = std::dynamic_pointer_cast<OutputComputePass>(s_AllPasses[ImagePassType::OutputCompute]);
    outputPass->SetBloomType(bloomType);

    s_ActiveBloomPassType = bloomType;
}

void ImageEditor::InitializeImageEditor(const std::string &inputDirectory,
                                        const std::string &outputDirectory,
                                        const std::string &settingsFileName)
{
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/Linearize.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/MultiPassBloom.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/RadialBloom.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/Sobel.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/RadialBlur.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/ContrastAdjust.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/ChromaticAberration.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/BarrelDistortion.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/Vignette.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/Sharpen.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/HSVAdjust.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/post_fx/OutputPass.glsl");
    askygg::ShaderLibrary::Load("assets/shaders/DisplayTexture.glsl");
    InitializeOutputDisplayPass();

    s_SettingsFileName = settingsFileName;
    s_InputDirectory = inputDirectory;
    s_OutputDirectory = outputDirectory;

    s_AllPasses[ImagePassType::Linearize] = askygg::CreateRef<LinearizePass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::BarrelDistortion] = askygg::CreateRef<BarrelDistortionPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::RadialBloom] = askygg::CreateRef<RadialBloomPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::MultiPassBloom] = askygg::CreateRef<MultiPassBloomPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::RadialBloom] = askygg::CreateRef<RadialBloomPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::ChromaticAberration] = askygg::CreateRef<ChromaticAberrationPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::ContrastBrightness] = askygg::CreateRef<ContrastBrightnessPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::HueShift] = askygg::CreateRef<HSVAdjustmentPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::RadialBlur] = askygg::CreateRef<RadialBlurPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::Sharpen] = askygg::CreateRef<SharpenPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::Sobel] = askygg::CreateRef<SobelPass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::Vignette] = askygg::CreateRef<VignettePass>(s_SettingsFileName);
    s_AllPasses[ImagePassType::OutputCompute] = askygg::CreateRef<OutputComputePass>(s_SettingsFileName);
    for(auto [passType, pass] : s_AllPasses)
    {
        pass->Initialize();
        pass->Load();
    }

    YAML::Node config = YAML::LoadFile(settingsFileName);
    std::vector<std::string> configPassOrder = config["PassOrder"] ? config["PassOrder"].as<std::vector<std::string>>() : GetDefaultPassOrderToString();
    int bloomTypeInt = config["Bloom Pass Type"] ? config["Bloom Pass Type"].as<int>() : 1;
    auto bloomType = static_cast<BloomType>(bloomTypeInt);
    SetBloomPass(bloomType);

    s_OrderedPassTypes.push_back(ImagePassType::Linearize);
    for(const auto& passString : configPassOrder)
        s_OrderedPassTypes.push_back(ImagePass::ImagePassTypeFromString(passString));
    s_OrderedPassTypes.push_back(ImagePassType::OutputCompute);
}

void SortDirectoryEntries(const std::string &directoryPath,
                          std::vector<std::filesystem::directory_entry> &entries)
{
    YGG_ASSERT(!std::filesystem::is_empty(directoryPath),
               "No input images in '{}' - please add at least one image before starting askygg.",
               directoryPath);

    for (const auto &entry: std::filesystem::directory_iterator(directoryPath))
        entries.push_back(entry);

    std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b)
    {
        return a.path().filename().string() < b.path().filename().string();
    });
}

void ImageEditor::HeadlessProcessDirectory()
{
    askygg::ScopedTimer timer("Process Image Directory", askygg::ScopedTimer::Unit::Minutes);

    askygg::Texture2DSpecification fileTexSpec = {
            askygg::ImageUtils::WrapMode::Repeat,
            askygg::ImageUtils::WrapMode::Repeat,
            askygg::ImageUtils::FilterMode::Linear,
            askygg::ImageUtils::FilterMode::Linear,
            askygg::ImageUtils::ImageInternalFormat::FromImage,
            askygg::ImageUtils::ImageDataLayout::FromImage,
            askygg::ImageUtils::ImageDataType::UByte,
    };

    std::vector<std::filesystem::directory_entry> entries;
    SortDirectoryEntries(s_InputDirectory, entries);

    for (const auto &entry: entries)
    {
        const std::string &filePath = entry.path().string();
        if (std::filesystem::is_regular_file(entry.status()))
        {
            auto *texture = new askygg::Texture2D(filePath, fileTexSpec);
            SaveTexture(*texture, s_OutputDirectory, false);
            delete texture;
        }
    }

    timer.Stop();
    long long int seconds = timer.GetNanoSeconds() * 1e-9;
    int numProcessed = entries.size();
    float processedPerSecond = static_cast<float>(numProcessed) / static_cast<float>(seconds);
    constexpr float PreviousProcessedPerMinute = 81.0f;
    YGG_LOG_INFO("{} images/s, {} images/min, {}x faster", processedPerSecond, processedPerSecond * 60.0f, processedPerSecond * 60.0f / PreviousProcessedPerMinute);
}

void ImageEditor::LoadTextureSet(const std::string &directoryPath)
{
    YGG_ASSERT(!std::filesystem::is_empty(directoryPath),
               "No input images in '{}' - please add at least one image before starting askygg.",
               directoryPath);

    askygg::Texture2DSpecification fileTexSpec = {
            askygg::ImageUtils::WrapMode::Repeat,
            askygg::ImageUtils::WrapMode::Repeat,
            askygg::ImageUtils::FilterMode::Linear,
            askygg::ImageUtils::FilterMode::Linear,
            askygg::ImageUtils::ImageInternalFormat::FromImage,
            askygg::ImageUtils::ImageDataLayout::FromImage,
            askygg::ImageUtils::ImageDataType::UByte,
    };

    std::vector<std::filesystem::directory_entry> entries;
    SortDirectoryEntries(directoryPath, entries);

    for (const auto &entry: entries)
    {
        const std::string &filePath = entry.path().string();
        if (std::filesystem::is_regular_file(entry.status()))
        {
            uint32_t textureID = askygg::TextureLibrary::LoadTexture2D(fileTexSpec, filePath)->GetID();
            s_TextureSet.push_back(textureID);
        }
    }
}

void ImageEditor::InitializeOutputDisplayPass()
{
    askygg::FramebufferSpecification OutputPassFBOSpec;
    const auto &Window = askygg::Application::GetWindow();
    OutputPassFBOSpec.AttachmentSpecification = {askygg::FramebufferTextureFormat::RGBA8};
    OutputPassFBOSpec.Width = Window.GetWidth();
    OutputPassFBOSpec.Height = Window.GetHeight();

    askygg::RenderPassSpecification DisplayPassSpec;
    DisplayPassSpec.PassFramebuffer = askygg::Framebuffer::Create(OutputPassFBOSpec);
    DisplayPassSpec.PassMaterial = askygg::CreateRef<askygg::Material>(
            "Display Texture Pass Material", askygg::ShaderLibrary::Get("DisplayTexture"));

    s_DisplayRenderPass = askygg::CreateRef<askygg::RenderPass>(DisplayPassSpec);
}

void ImageEditor::SubmitOutputDisplayPass(uint32_t displayTextureID)
{
    askygg::Renderer::BeginPass(s_DisplayRenderPass);
    const askygg::TextureUniform displayTextureUniform{displayTextureID, 0, 1};
    s_DisplayRenderPass->GetSpecification().PassMaterial->Set<askygg::TextureUniform>("u_Texture", displayTextureUniform);
    askygg::Renderer::DrawFullScreenQuad(s_DisplayRenderPass->GetSpecification().PassMaterial);
    askygg::Renderer::EndPass(s_DisplayRenderPass);
}

float Execute(const std::function<void()>& operation, bool profile)
{
    if (profile)
    {
        askygg::OpenGLFuncTimer timer;
        return timer.ProfileFn(operation);
    }

    operation();
    return -1.0f;
}

void ImageEditor::SubmitPipeline(const glm::vec2 &targetSize, uint32_t targetTextureID, bool display, bool profile)
{
    if (askygg::ShaderLibrary::IsEmpty())
        return;

    askygg::Renderer::BeginScene(targetSize);

    auto linearizePass = s_AllPasses[ImagePassType::Linearize];
    linearizePass->OnResize(targetSize);
    auto linearizeSubmitFn =
            [object = linearizePass, targetTextureID]
            {
                object->Submit(targetTextureID);
            };
    s_PassExecutionTime[ImagePassType::Linearize] = Execute(linearizeSubmitFn, profile);
    uint32_t linearizeOutput = linearizePass->GetOutputID();

    if(s_ActiveBloomPassType != BloomType::None)
    {
        s_ActiveBloomPass->OnResize(targetSize);
        ImagePassType bloomImagePassType = s_ActiveBloomPassType == BloomType::Radial ? ImagePassType::RadialBloom : ImagePassType::MultiPassBloom;
        auto bloomSubmitFn =
                [object = s_ActiveBloomPass, linearizeOutput]
                {
                    object->Submit(linearizeOutput);
                };
        s_PassExecutionTime[bloomImagePassType] = Execute(bloomSubmitFn, profile);
    }

    for(int i = 1; i < s_OrderedPassTypes.size(); i++)
    {
        auto currentPassType = s_OrderedPassTypes[i];
        auto currentPass = s_AllPasses[currentPassType];
        currentPass->OnResize(targetSize);

        uint32_t previousPassOutput = s_AllPasses[s_OrderedPassTypes[i - 1]]->GetOutputID();

        auto submitFn =
                [object = currentPass, previousPassOutput]
                {
                    object->Submit(previousPassOutput);
                };
        s_PassExecutionTime[currentPassType] = Execute(submitFn, profile);
    }

    if (display)
    {
        uint32_t outputComputeOutID = s_AllPasses[ImagePassType::OutputCompute]->GetOutputID();
        s_DisplayRenderPass->GetSpecification().PassFramebuffer->Resize(targetSize.x, targetSize.y);
        SubmitOutputDisplayPass(outputComputeOutID);
    }
    askygg::Renderer::EndScene();
}

void ImageEditor::DrawActiveTexture()
{
    if (s_TextureSet.empty())
        return;
    auto &activeTexture = askygg::TextureLibrary::Get2DFromID(s_TextureSet[s_ActiveTextureIndex]);
    auto activeTextureSize = glm::vec2(activeTexture.GetWidth(), activeTexture.GetHeight());

    if(s_DisplayUnprocessedInput)
    {
        askygg::Renderer::BeginScene(activeTextureSize);
        s_DisplayRenderPass->GetSpecification().PassFramebuffer->Resize(activeTextureSize.x, activeTextureSize.y);
        SubmitOutputDisplayPass(s_TextureSet[s_ActiveTextureIndex]);
        askygg::Renderer::EndScene();
    }
    else
    {
        SubmitPipeline(activeTextureSize, s_TextureSet[s_ActiveTextureIndex], true, true);
    }
}

void ImageEditor::SaveTexture(const askygg::Texture2D &texture,
                              const std::string &outputDirectory,
                              bool profile)
{
    glm::vec2 textureSize{texture.GetWidth(), texture.GetHeight()};
    SubmitPipeline(textureSize, texture.GetID(), false, profile);

    std::filesystem::path path(texture.GetName());
    // Removes the previous extension - keeps just the name.
    std::string name = path.stem().string();

    std::string filePath = outputDirectory + name + ".jpeg";

    uint32_t outputID = s_AllPasses[ImagePassType::OutputCompute]->GetOutputID();
    auto &outputTexture = askygg::TextureLibrary::Get2DFromID(outputID);
    outputTexture.Save(filePath, true);
}

void ImageEditor::SavePassOrder()
{
    YAML::Node existingConfig;
    existingConfig = YAML::LoadFile(s_SettingsFileName);
    std::vector<std::string> orderedPassesToString;
    for(int i = 1; i < s_OrderedPassTypes.size() - 1; i++)
    {
        std::string passToString = ImagePass::ImagePassTypeToString(s_OrderedPassTypes[i]);
        orderedPassesToString.push_back(passToString);
    }
    existingConfig["PassOrder"] = orderedPassesToString;
    std::ofstream outFile(s_SettingsFileName);
    outFile << existingConfig;
}

void DrawDisabledButton(const std::string& label)
{
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
    ImGui::Button(label.c_str());
    ImGui::PopItemFlag();
    ImGui::PopStyleVar();
}

void ImageEditor::DrawImageEditorUI()
{
    ImGui::Begin("Pass Inspector");
    // Draw the output compute pass first for convenience.
    auto outputComputePass = s_AllPasses[ImagePassType::OutputCompute];
    outputComputePass->DrawUI();
    if(s_ActiveBloomPassType != BloomType::None && s_ActiveBloomPass != nullptr)
        s_ActiveBloomPass->DrawUI();
    // Skip the last element - which will be the output compute pass.
    for(int i = 0; i < s_OrderedPassTypes.size() - 1; i++)
        s_AllPasses[s_OrderedPassTypes[i]]->DrawUI();
    ImGui::End();

    ImGui::Begin("Capture Controls");
    if (ImGui::Button("Previous"))
    {
        if (s_ActiveTextureIndex == 0)
            s_ActiveTextureIndex = s_TextureSet.size() - 1;
        else
            s_ActiveTextureIndex--;
    }
    ImGui::SameLine(75);
    if (ImGui::Button("Next"))
    {
        s_ActiveTextureIndex = (s_ActiveTextureIndex + 1) % s_TextureSet.size();
    }
    ImGui::SameLine(150);

    std::string toggleOriginalText = s_DisplayUnprocessedInput ? "Show Processed" : "Show Original";
    if (ImGui::Button(toggleOriginalText.c_str()))
    {
        s_DisplayUnprocessedInput = !s_DisplayUnprocessedInput;
    }

    if (ImGui::CollapsingHeader("Capture Frame"), ImGuiTreeNodeFlags_DefaultOpen)
    {
        if (ImGui::Button("Capture Current"))
        {
            const askygg::Texture2D &activeTexture =
                    askygg::TextureLibrary::Get2DFromID(s_TextureSet[s_ActiveTextureIndex]);
            SaveTexture(activeTexture, s_OutputDirectory);
        }

        if (ImGui::Button("Capture All"))
        {
            std::chrono::high_resolution_clock::time_point start =
                    std::chrono::high_resolution_clock::now();
            for (unsigned int i: s_TextureSet)
            {
                const askygg::Texture2D &activeTexture =
                        askygg::TextureLibrary::Get2DFromID(s_TextureSet[i]);
                SaveTexture(activeTexture, s_OutputDirectory);
            }

            std::chrono::high_resolution_clock::time_point end =
                    std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> time_span = end - start;
            YGG_LOG_INFO("'Capture All' command completed in: {} ms", std::to_string(time_span.count()));
        }
    }
    ImGui::End();

    ImGui::Begin("Performance Monitor");
    if (ImGui::TreeNode("Image Pass Types"))
    {
        constexpr float updateSeconds = 3;
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedSeconds = std::chrono::duration<double>(currentTime - s_LastSortTime).count();
        if (elapsedSeconds >= updateSeconds)
        {
            s_SortedExecutionTimes.assign(s_PassExecutionTime.begin(), s_PassExecutionTime.end());

            std::sort(s_SortedExecutionTimes.begin(), s_SortedExecutionTimes.end(),
                      [](const auto &a, const auto &b)
                      { return a.second > b.second; });

            s_LastSortTime = currentTime;
        }

        ImagePassType bloomImagePassType = ImagePass::ImagePassTypeFromBloomType(s_ActiveBloomPassType);
        for (auto [passType, time]: s_SortedExecutionTimes)
        {
            auto it = std::find(s_OrderedPassTypes.begin(), s_OrderedPassTypes.end(), passType);
            if(passType == bloomImagePassType || it != s_OrderedPassTypes.end())
                ImGui::BulletText("%s: %fms", ImagePass::ImagePassTypeToString(passType).c_str(), time);
        }
        ImGui::TreePop();
    }

    ImGui::End();

    ImGui::Begin("Execution Definition");

    static const char* items[] = { "None", "Radial", "Multi-Pass" };
    static int		   currentItem = static_cast<int>(s_ActiveBloomPassType);

    if (ImGui::Combo("Bloom Type", &currentItem, items, IM_ARRAYSIZE(items)))
    {
        auto type = static_cast<BloomType>(currentItem);

        SetBloomPass(type);
        // Update the config with the new bloom type.
        YAML::Node existingConfig;
        existingConfig = YAML::LoadFile(s_SettingsFileName);
        existingConfig["Bloom Pass Type"] = static_cast<int>(type);
        std::ofstream outFile(s_SettingsFileName);
        outFile << existingConfig;
    }

    float windowWidth = ImGui::GetContentRegionAvail().x;
    float panelWidth = windowWidth / 2.0f;

    ImGui::BeginChild("ActivePasses", ImVec2(panelWidth, 0), true);
    {
        DrawDisabledButton(ImagePass::ImagePassTypeToString(s_OrderedPassTypes[0]));

        for (size_t i = 1; i < s_OrderedPassTypes.size() - 1; ++i)
        {
            std::string typeString = ImagePass::ImagePassTypeToString(s_OrderedPassTypes[i]);
            ImGui::Button(typeString.c_str());

            if(ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("ACTIVE_PASS", &s_OrderedPassTypes[i], sizeof(ImagePassType));
                ImGui::EndDragDropSource();
            }

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ACTIVE_PASS"))
                {
                    IM_ASSERT(payload->DataSize == sizeof(size_t));
                    ImagePassType swapType = *(const ImagePassType*)payload->Data;
                    auto it = std::find(s_OrderedPassTypes.begin(), s_OrderedPassTypes.end(), swapType);
                    if (it != s_OrderedPassTypes.end())
                    {
                        size_t index = std::distance(s_OrderedPassTypes.begin(), it);
                        std::swap(s_OrderedPassTypes[i], s_OrderedPassTypes[index]);
                        SavePassOrder();
                    }
                }
                ImGui::EndDragDropTarget();
            }
        }

        DrawDisabledButton(ImagePass::ImagePassTypeToString(s_OrderedPassTypes[s_OrderedPassTypes.size() - 1]));

        // Drop inactive pass types here
        ImGui::InvisibleButton("DropAreaActive", ImVec2(-1, -1));
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("INACTIVE_PASS"))
            {
                IM_ASSERT(payload->DataSize == sizeof(ImagePassType));
                ImagePassType newType = *(const ImagePassType*)payload->Data;
                s_OrderedPassTypes.insert(s_OrderedPassTypes.end() - 1, newType);
                SavePassOrder();
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Inactive Passes Panel
    ImGui::BeginChild("InactivePasses", ImVec2(panelWidth - panelWidth * 0.025f, 0), true);
    {
        for (const auto& type : ImagePass::GetAllBasicImagePassTypes())
        {
            if (std::find(s_OrderedPassTypes.begin(), s_OrderedPassTypes.end(), type) == s_OrderedPassTypes.end())
            {
                ImGui::Button(ImagePass::ImagePassTypeToString(type).c_str());
                if (ImGui::BeginDragDropSource())
                {
                    ImGui::SetDragDropPayload("INACTIVE_PASS", &type, sizeof(ImagePassType));
                    ImGui::EndDragDropSource();
                }
            }
        }

        ImGui::InvisibleButton("DropAreaInactive", ImVec2(-1, -1));

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ACTIVE_PASS"))
            {
                IM_ASSERT(payload->DataSize == sizeof(ImagePassType));
                ImagePassType newType = *static_cast<const ImagePassType*>(payload->Data);
                if (auto it = std::find(s_OrderedPassTypes.begin(), s_OrderedPassTypes.end(), newType); it != s_OrderedPassTypes.end())
                    s_OrderedPassTypes.erase(it);
                SavePassOrder();
            }
            ImGui::EndDragDropTarget();
        }
    }
    ImGui::EndChild();

    ImGui::End();
}

const askygg::Ref<askygg::Framebuffer> &ImageEditor::GetOutputFBO()
{
    return s_DisplayRenderPass->GetSpecification().PassFramebuffer;
}

void ImageEditor::ShutdownImageEditor()
{
    s_AllPasses.clear();
}
