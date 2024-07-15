#include "ContrastBrightnessPass.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <utility>

ContrastBrightnessPass::ContrastBrightnessPass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void ContrastBrightnessPass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("ContrastAdjust");
	const auto& Window = askygg::Application::GetWindow();

	askygg::Texture2DSpecification ContrastTextureSpec = {
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

	ContrastTextureSpec.Name = m_OutputName;
	m_ContrastBrightnessOutput = askygg::CreateRef<askygg::Texture2D>(ContrastTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_ContrastBrightnessOutput);
}

void ContrastBrightnessPass::Submit(uint32_t textureID)
{
	m_Shader->Bind();
	askygg::Texture2D::BindTextureIDToSamplerSlot(0, textureID);
	m_Shader->UploadUniformInt("u_Texture", 0);
	m_Shader->UploadUniformFloat("u_ContrastStrength", m_Settings.ContrastStrength);
	m_Shader->UploadUniformFloat("u_Brightness", m_Settings.Brightness);

	glm::vec2 textureSize = { m_ContrastBrightnessOutput->GetWidth(),
		m_ContrastBrightnessOutput->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	m_ContrastBrightnessOutput->BindToImageSlot(0, 0,
		askygg::ImageUtils::TextureAccessLevel::WriteOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void ContrastBrightnessPass::DrawUI()
{
	if (ImGui::CollapsingHeader("Contrast & Brightness Settings"))
	{
		askygg::UI::UIFloat::Draw("Contrast Strength", &m_Settings.ContrastStrength);
		askygg::UI::UIFloat::Draw("Brightness", &m_Settings.Brightness);

		if (ImGui::Button("Save Contrast & Brightness Settings"))
			Save();
	}
}

void ContrastBrightnessPass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["Contrast & Brightness"]["Contrast Strength"] = m_Settings.ContrastStrength;
	existingConfig["Contrast & Brightness"]["Brightness"] = m_Settings.Brightness;
	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void ContrastBrightnessPass::Load()
{
	YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
	m_Settings.ContrastStrength =
		config["Contrast & Brightness"]["Contrast Strength"]
		? config["Contrast & Brightness"]["Contrast Strength"].as<float>()
		: 1.0f;
	m_Settings.Brightness = config["Contrast & Brightness"]["Brightness"]
		? config["Contrast & Brightness"]["Brightness"].as<float>()
		: 0.0f;
}

void ContrastBrightnessPass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;
	ImagePass::OnResize(targetSize);
	m_ContrastBrightnessOutput->Resize(targetSize.x, targetSize.y);
}
