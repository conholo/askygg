#include "SobelPass.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <utility>

SobelPass::SobelPass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void SobelPass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("Sobel");
	const auto& Window = askygg::Application::GetWindow();

	askygg::Texture2DSpecification SobelTextureSpec = {
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

	SobelTextureSpec.Name = m_OutputName;
	m_SobelOutput = askygg::CreateRef<askygg::Texture2D>(SobelTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_SobelOutput);
}

void SobelPass::Submit(uint32_t textureID)
{
	m_Shader->Bind();
	askygg::Texture2D::BindTextureIDToSamplerSlot(0, textureID);
	m_Shader->UploadUniformInt("u_Texture", 0);
	m_Shader->UploadUniformFloat("u_Strength", m_Settings.SobelStrength);
	m_Shader->UploadUniformFloat("u_Threshold", m_Settings.Threshold);

	glm::vec2 textureSize = { m_SobelOutput->GetWidth(), m_SobelOutput->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	m_SobelOutput->BindToImageSlot(0, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void SobelPass::DrawUI()
{
	if (ImGui::CollapsingHeader("Sobel Settings"))
	{
		askygg::UI::UIFloat::Draw("Sobel Strength", &m_Settings.SobelStrength);
		askygg::UI::UIFloat::Draw("Threshold", &m_Settings.Threshold);

		if (ImGui::Button("Save Sobel Settings"))
			Save();
	}
}

void SobelPass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["Sobel"]["Sobel Strength"] = m_Settings.SobelStrength;
	existingConfig["Sobel"]["Sobel Threshold"] = m_Settings.Threshold;

	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void SobelPass::Load()
{
    YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
    m_Settings.SobelStrength = config["Sobel"]["Sobel Strength"] ? config["Sobel"]["Sobel Strength"].as<float>() : 0.0f;
    m_Settings.Threshold = config["Sobel"]["Sobel Threshold"] ? config["Sobel"]["Sobel Threshold"].as<float>() : 0.0f;
}

void SobelPass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;

	ImagePass::OnResize(targetSize);
	m_SobelOutput->Resize(targetSize.x, targetSize.y);
}
