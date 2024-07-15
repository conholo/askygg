#include "RadialBloomPass.h"
#include "core/Application.h"
#include "imgui.h"
#include "ui/PropertyDrawer.h"
#include <utility>
#include <fstream>


RadialBloomPass::RadialBloomPass(std::string settingsFilePath)
        : ImagePass(std::move(settingsFilePath)) {}

void RadialBloomPass::Initialize()
{
    m_Shader = askygg::ShaderLibrary::Get("RadialBloom");
    const auto& Window = askygg::Application::GetWindow();

    askygg::Texture2DSpecification bloomEdgeLightsSpec = {
            askygg::ImageUtils::WrapMode::ClampToEdge,
            askygg::ImageUtils::WrapMode::ClampToEdge,
            askygg::ImageUtils::FilterMode::Linear,
            askygg::ImageUtils::FilterMode::Linear,
            askygg::ImageUtils::ImageInternalFormat::RGBA32F,
            askygg::ImageUtils::ImageDataLayout::RGBA,
            askygg::ImageUtils::ImageDataType::Float,
            Window.GetWidth(),
            Window.GetHeight()
    };

    bloomEdgeLightsSpec.Name = m_OutputName;
    m_Output = askygg::CreateRef<askygg::Texture2D>(bloomEdgeLightsSpec);
    askygg::TextureLibrary::AddTexture2D(m_Output);

    bloomEdgeLightsSpec.Name = "Bloom Intermediate Output";
    m_Intermediate = askygg::CreateRef<askygg::Texture2D>(bloomEdgeLightsSpec);
    askygg::TextureLibrary::AddTexture2D(m_Intermediate);

    bloomEdgeLightsSpec.Name = "Bloom Emitter Extraction Output";
    m_BloomEmitterExtractionOutput = askygg::CreateRef<askygg::Texture2D>(bloomEdgeLightsSpec);
    askygg::TextureLibrary::AddTexture2D(m_BloomEmitterExtractionOutput);
}

void RadialBloomPass::Submit(uint32_t textureID)
{
    glm::vec2 textureSize = {m_Output->GetWidth(), m_Output->GetHeight() };
    auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
    auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

    m_Shader->Bind();

    // Extraction Pass
    {
        askygg::Texture2D::BindTextureIDToImageSlot(textureID, 0, 0, askygg::ImageUtils::TextureAccessLevel::ReadOnly, askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
        m_BloomEmitterExtractionOutput->BindToImageSlot(1, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly, askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
        m_Shader->UploadUniformInt("u_Mode", 0);
        m_Shader->UploadUniformFloat("u_LuminanceThreshold", m_Settings.LuminanceThreshold);
        m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
        m_Shader->EnableShaderImageAccessBarrierBit();
        askygg::Texture2D::ClearBinding();
    }
    // Horizontal Blur Pass
    {
        m_BloomEmitterExtractionOutput->BindToImageSlot(0, 0, askygg::ImageUtils::TextureAccessLevel::ReadOnly, askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
        m_Intermediate->BindToImageSlot(1, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly, askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
        m_Shader->UploadUniformInt("u_Mode", 1);
        m_Shader->UploadUniformFloat("u_Amplitude", m_Settings.BloomAmplitude);
        m_Shader->UploadUniformFloat("u_SigmaScaleFactor", m_Settings.SigmaScaleFactor);
        m_Shader->UploadUniformFloat("u_BlurColorWeight", m_Settings.BlurColorWeight);
        m_Shader->UploadUniformInt("u_BloomPixelRadius", m_Settings.BloomRadiusPixels);
        m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
        m_Shader->EnableShaderImageAccessBarrierBit();
        askygg::Texture2D::ClearBinding();
    }

    // Vertical Blur Pass
    {
        m_Intermediate->BindToImageSlot(0, 0, askygg::ImageUtils::TextureAccessLevel::ReadOnly, askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
        m_Output->BindToImageSlot(1, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly, askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
        m_Shader->UploadUniformInt("u_Mode", 2);
        m_Shader->UploadUniformFloat("u_Amplitude", m_Settings.BloomAmplitude);
        m_Shader->UploadUniformFloat("u_SigmaScaleFactor", m_Settings.SigmaScaleFactor);
        m_Shader->UploadUniformFloat("u_BlurColorWeight", m_Settings.BlurColorWeight);
        m_Shader->UploadUniformInt("u_BloomPixelRadius", m_Settings.BloomRadiusPixels);
        m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
        m_Shader->EnableShaderImageAccessBarrierBit();
        askygg::Texture2D::ClearBinding();
    }
    m_Shader->Unbind();
}

void RadialBloomPass::DrawUI()
{
    if (ImGui::CollapsingHeader("Radial Bloom Settings"))
    {
        askygg::UI::UIInt::DrawDragInt("Pixel Radius", &m_Settings.BloomRadiusPixels, 0.1f, 0, 20);
        askygg::UI::UIFloat::Draw("Luminance Threshold", &m_Settings.LuminanceThreshold);
        askygg::UI::UIFloat::Draw("Amplitude", &m_Settings.BloomAmplitude);
        askygg::UI::UIFloat::Draw("Sigma Scale Factor", &m_Settings.SigmaScaleFactor);
        askygg::UI::UIFloat::Draw("Blur Color Weight", &m_Settings.BlurColorWeight);
        if (ImGui::Button("Save Radial Bloom Settings"))
            Save();
    }
}

void RadialBloomPass::Save()
{
    YAML::Node existingConfig;
    existingConfig = YAML::LoadFile(m_SettingsFilePath);
    existingConfig["RadialBloom"]["Amplitude"] = m_Settings.BloomAmplitude;
    existingConfig["RadialBloom"]["Pixel Radius"] = m_Settings.BloomRadiusPixels;
    existingConfig["RadialBloom"]["Luminance Threshold"] = m_Settings.LuminanceThreshold;
    existingConfig["RadialBloom"]["Sigma Scale Factor"] = m_Settings.SigmaScaleFactor;
    existingConfig["RadialBloom"]["Blur Color Weight"] = m_Settings.BlurColorWeight;
    std::ofstream outFile(m_SettingsFilePath);
    outFile << existingConfig;
}

void RadialBloomPass::Load()
{
    YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
    m_Settings.BloomRadiusPixels =
            config["RadialBloom"]["Pixel Radius"] ? config["RadialBloom"]["Pixel Radius"].as<int>() : 1;
    m_Settings.BloomAmplitude =
            config["RadialBloom"]["Amplitude"] ? config["RadialBloom"]["Amplitude"].as<float>() : 2.0f;
    m_Settings.LuminanceThreshold =
            config["RadialBloom"]["Luminance Threshold"] ? config["RadialBloom"]["Luminance Threshold"].as<float>() : 1.0f;
    m_Settings.SigmaScaleFactor =
            config["RadialBloom"]["Sigma Scale Factor"] ? config["RadialBloom"]["Sigma Scale Factor"].as<float>() : 3.0f;
    m_Settings.BlurColorWeight =
            config["RadialBloom"]["Blur Color Weight"] ? config["RadialBloom"]["Blur Color Weight"].as<float>() : 0.5f;
}

void RadialBloomPass::OnResize(const glm::vec2& targetSize)
{
    if (m_OutputSize == targetSize)
        return;
    ImagePass::OnResize(targetSize);
    m_Output->Resize(targetSize.x, targetSize.y);
    m_Intermediate->Resize(targetSize.x, targetSize.y);
    m_BloomEmitterExtractionOutput->Resize(targetSize.x, targetSize.y);
}
