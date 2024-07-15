#include <fstream>
#include <chrono>
#include "OutputComputePass.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include "imgui.h"
#include "platform/PlatformPath.h"

OutputComputePass::OutputComputePass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void OutputComputePass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("OutputPass");
	const auto& Window = askygg::Application::GetWindow();

    askygg::Texture2DSpecification fileTextureSpec = {
            askygg::ImageUtils::WrapMode::Repeat,
            askygg::ImageUtils::WrapMode::Repeat,
            askygg::ImageUtils::FilterMode::Linear,
            askygg::ImageUtils::FilterMode::Linear,
            askygg::ImageUtils::ImageInternalFormat::FromImage,
            askygg::ImageUtils::ImageDataLayout::FromImage,
            askygg::ImageUtils::ImageDataType::UByte,
    };

    fileTextureSpec.Name = "Sensor Noise Patch";
    const std::string sensorNoisePath = GetExecutablePath() + "/assets/textures/sensor-noise.jpg";
    m_SensorNoisePatchTexture = askygg::TextureLibrary::LoadTexture2D(fileTextureSpec, sensorNoisePath);

    fileTextureSpec.Name = "MultiPassBloom Dirt Mask";
    const std::string bloomDirtMaskPath = GetExecutablePath() + "/assets/textures/dirt-mask.png";
    m_BloomDirtTexture = askygg::TextureLibrary::LoadTexture2D(fileTextureSpec, bloomDirtMaskPath);

    askygg::Texture2DSpecification OutputComputeTextureSpec = {
		askygg::ImageUtils::WrapMode::ClampToEdge,
		askygg::ImageUtils::WrapMode::ClampToEdge,
		askygg::ImageUtils::FilterMode::Linear,
		askygg::ImageUtils::FilterMode::Linear,
		askygg::ImageUtils::ImageInternalFormat::RGBA8,
		askygg::ImageUtils::ImageDataLayout::RGBA,
		askygg::ImageUtils::ImageDataType::UByte,
		Window.GetWidth(),
		Window.GetHeight()
	};

    OutputComputeTextureSpec.Name = m_OutputName;
	m_ByteOutput = askygg::CreateRef<askygg::Texture2D>(OutputComputeTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_ByteOutput);

    m_Start = std::chrono::high_resolution_clock::now();
}


void OutputComputePass::Submit(uint32_t textureID)
{
    auto current_time = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration<float>(current_time - m_Start).count();

    m_Shader->Bind();
    std::string bloomOutputName = m_Settings.ActiveBloomType == BloomType::Radial ? "Radial Bloom Output" : "Multi-Pass Bloom Output";
	uint32_t bloomTextureID = askygg::TextureLibrary::GetIDFromName(bloomOutputName);

	askygg::TextureLibrary::BindTextureToSlot(bloomTextureID, 0);
	m_Shader->UploadUniformInt("u_BloomTexture", 0);
	askygg::TextureLibrary::BindTextureToSlot(m_BloomDirtTexture->GetID(), 1);
	m_Shader->UploadUniformInt("u_BloomDirtTexture", 1);
	askygg::TextureLibrary::BindTextureToSlot(m_SensorNoisePatchTexture->GetID(), 2);
	m_Shader->UploadUniformInt("u_SensorNoisePatchTexture", 2);

    m_Shader->UploadUniformFloat("u_Exposure", m_Settings.Exposure);
    m_Shader->UploadUniformFloat("u_WhitePoint", m_Settings.WhitePoint);
    m_Shader->UploadUniformInt("u_Tonemapper", static_cast<int>(m_Settings.Tonemap));

    m_Shader->UploadUniformFloat("u_NoiseAlpha", m_Settings.NoiseAlpha);
    m_Shader->UploadUniformFloat("u_NoiseBeta", m_Settings.NoiseBeta);
    m_Shader->UploadUniformFloat("u_NoiseGamma", m_Settings.NoiseGamma);
    m_Shader->UploadUniformFloat("u_NoiseFrequency", m_Settings.NoiseFrequency);
    m_Shader->UploadUniformFloat("u_NoiseAmplitude", m_Settings.NoiseAmplitude);
    m_Shader->UploadUniformFloat("u_Time", elapsed);

    m_Shader->UploadUniformInt("u_BloomType", static_cast<int>(m_Settings.ActiveBloomType) * -1);
    m_Shader->UploadUniformInt("u_BloomEnabled", m_Settings.ActiveBloomType != BloomType::None ? 1 : 0);
	m_Shader->UploadUniformFloat("u_BloomIntensity", m_Settings.BloomIntensity);
	m_Shader->UploadUniformFloat("u_BloomDirtIntensity", m_Settings.BloomDirtIntensity);
	m_Shader->UploadUniformFloat("u_UpsampleRadius", m_Settings.BloomUpsampleRadius);
	m_Shader->UploadUniformFloat("u_UpsampleTightenFactor", m_Settings.BloomUpsampleTightenFactor);

	glm::vec2 textureSize = { m_ByteOutput->GetWidth(), m_ByteOutput->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	auto& fInputTexture = askygg::TextureLibrary::Get2DFromID(textureID);
	fInputTexture.BindToImageSlot(0, 0, askygg::ImageUtils::TextureAccessLevel::ReadOnly,askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_ByteOutput->BindToImageSlot(1, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,askygg::ImageUtils::TextureShaderDataFormat::RGBA8);
	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void OutputComputePass::DrawUI()
{
	if (ImGui::CollapsingHeader("Composite Pass Settings"))
	{
		askygg::UI::UIFloat::Draw("Exposure", &m_Settings.Exposure);

        if(ImGui::TreeNodeEx("Tonemap Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            static const char* items[] = { "None", "ACES", "Reinhard", "ReinhardExtended" };
            static int		   currentItem = static_cast<int>(m_Settings.Tonemap);

            if (ImGui::Combo("Tonemapper", &currentItem, items, IM_ARRAYSIZE(items)))
                m_Settings.Tonemap = static_cast<TonemappingType>(currentItem);
            if (m_Settings.Tonemap == TonemappingType::Reinhard || m_Settings.Tonemap == TonemappingType::ReinhardExtended)
                askygg::UI::UIFloat::Draw("White Point", &m_Settings.WhitePoint);

            ImGui::TreePop();
        }

        if(ImGui::TreeNodeEx("Sensor Noise Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            askygg::UI::UIFloat::Draw("Blend Alpha", &m_Settings.NoiseAlpha);
            askygg::UI::UIFloat::Draw("Blend Beta", &m_Settings.NoiseBeta);
            askygg::UI::UIFloat::Draw("Blend Gamma", &m_Settings.NoiseGamma);
            askygg::UI::UIFloat::Draw("Offset Perlin Frequency", &m_Settings.NoiseFrequency);
            askygg::UI::UIFloat::Draw("Offset Perlin Amplitude", &m_Settings.NoiseAmplitude);
            ImGui::TreePop();
        }

        if (ImGui::TreeNodeEx("Bloom Composite Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (m_Settings.ActiveBloomType != BloomType::None)
            {
                askygg::UI::UIFloat::Draw("Bloom Intensity", &m_Settings.BloomIntensity);
                askygg::UI::UIFloat::Draw("Bloom Dirt Intensity", &m_Settings.BloomDirtIntensity);
            }

            if(m_Settings.ActiveBloomType == BloomType::MultiPass)
            {
                askygg::UI::UIFloat::Draw("Upsample Radius", &m_Settings.BloomUpsampleRadius);
                askygg::UI::UIFloat::Draw("Tighten Factor", &m_Settings.BloomUpsampleTightenFactor);
            }

            ImGui::TreePop();
        }

        if (ImGui::Button("Save Composite Pass Settings"))
			Save();
	}
}

void OutputComputePass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["Output"]["Exposure"] = m_Settings.Exposure;
	existingConfig["Output"]["Tonemapper"] = static_cast<int>(m_Settings.Tonemap);
	existingConfig["Output"]["WhitePoint"] = m_Settings.WhitePoint;

    existingConfig["Output"]["Sensor Noise Alpha"] = m_Settings.NoiseAlpha;
    existingConfig["Output"]["Sensor Noise Beta"] = m_Settings.NoiseBeta;
    existingConfig["Output"]["Sensor Noise Gamma"] = m_Settings.NoiseGamma;

    existingConfig["Output"]["Sensor Offset Perlin Frequency"] = m_Settings.NoiseFrequency;
    existingConfig["Output"]["Sensor Offset Perlin Amplitude"] = m_Settings.NoiseAmplitude;

    existingConfig["Output"]["Bloom Intensity"] = m_Settings.BloomIntensity;
    existingConfig["Output"]["Bloom Dirt Intensity"] = m_Settings.BloomDirtIntensity;
    existingConfig["Output"]["Bloom Upsample Radius"] = m_Settings.BloomUpsampleRadius;
    existingConfig["Output"]["Bloom Upsample Tighten Factor"] = m_Settings.BloomUpsampleTightenFactor;

	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void OutputComputePass::Load()
{
	YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
	m_Settings.Exposure = config["Output"]["Exposure"] ? config["Output"]["Exposure"].as<float>() : 1.0f;

	int tonemapInt = config["Output"]["Tonemapper"] ? config["Output"]["Tonemapper"].as<int>() : 0;
	m_Settings.Tonemap = static_cast<TonemappingType>(tonemapInt);
	m_Settings.WhitePoint = config["Output"]["WhitePoint"] ? config["Output"]["WhitePoint"].as<float>() : 1.0f;

	m_Settings.NoiseAlpha = config["Output"]["Sensor Noise Alpha"] ? config["Output"]["Sensor Noise Alpha"].as<float>() : 1.0f;
	m_Settings.NoiseBeta = config["Output"]["Sensor Noise Beta"] ? config["Output"]["Sensor Noise Beta"].as<float>() : 1.0f;
	m_Settings.NoiseGamma = config["Output"]["Sensor Noise Gamma"] ? config["Output"]["Sensor Noise Gamma"].as<float>() : 0.0f;

	m_Settings.NoiseFrequency = config["Output"]["Sensor Offset Perlin Frequency"] ? config["Output"]["Sensor Offset Perlin Frequency"].as<float>() : 1.0f;
	m_Settings.NoiseAmplitude = config["Output"]["Sensor Offset Perlin Amplitude"] ? config["Output"]["Sensor Offset Perlin Amplitude"].as<float>() : 1.0f;

    m_Settings.BloomIntensity = config["Output"]["Bloom Intensity"] ? config["Output"]["Bloom Intensity"].as<float>() : 1.0f;
    m_Settings.BloomDirtIntensity = config["Output"]["Bloom Dirt Intensity"] ? config["Output"]["Bloom Dirt Intensity"].as<float>() : 0.0f;
    m_Settings.BloomUpsampleRadius = config["Output"]["Bloom Upsample Radius"] ? config["Output"]["Bloom Upsample Radius"].as<float>() : 1.0f;
    m_Settings.BloomUpsampleTightenFactor = config["Output"]["Bloom Upsample Tighten Factor"] ? config["Output"]["Bloom Upsample Tighten Factor"].as<float>() : 16.0f;
}

void OutputComputePass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;

	ImagePass::OnResize(targetSize);
	m_ByteOutput->Resize(targetSize.x, targetSize.y);
}
