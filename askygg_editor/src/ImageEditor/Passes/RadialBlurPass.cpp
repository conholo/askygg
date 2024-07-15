#include "RadialBlurPass.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <utility>

RadialBlurPass::RadialBlurPass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void RadialBlurPass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("RadialBlur");
	const auto& Window = askygg::Application::GetWindow();

	askygg::Texture2DSpecification RadialBlurTextureSpec = {
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

	RadialBlurTextureSpec.Name = m_OutputName;
	m_RadialBlurOutput = askygg::CreateRef<askygg::Texture2D>(RadialBlurTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_RadialBlurOutput);
}

void RadialBlurPass::Submit(uint32_t textureID)
{
	m_Shader->Bind();
	askygg::Texture2D::BindTextureIDToSamplerSlot(0, textureID);
	m_Shader->UploadUniformInt("u_Texture", 0);
	m_Shader->UploadUniformInt("u_BlurSamples", m_Settings.BlurSamples);
	m_Shader->UploadUniformFloat("u_BlurStrength", m_Settings.BlurStrength);
	m_Shader->UploadUniformFloat2("u_BlurDirection",
		glm::vec2(m_Settings.BlurDirection.x, m_Settings.BlurDirection.y));

	glm::vec2 textureSize = { m_RadialBlurOutput->GetWidth(), m_RadialBlurOutput->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	m_RadialBlurOutput->BindToImageSlot(0, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void RadialBlurPass::DrawUI()
{
	if (ImGui::CollapsingHeader("Radial Blur Settings"))
	{
		askygg::UI::UIFloat::Draw("Blur Strength", &m_Settings.BlurStrength);
		askygg::UI::UIVector3::Draw("Blur Direction", &m_Settings.BlurDirection);
		askygg::UI::UIInt::DrawDragInt("Blur Samples", &m_Settings.BlurSamples, 1.0, 0.0, 1000.0);

		if (ImGui::Button("Save Radial Blur Settings"))
			Save();
	}
}

void RadialBlurPass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["Radial Blur"]["Blur Strength"] = m_Settings.BlurStrength;
	existingConfig["Radial Blur"]["Blur Samples"] = m_Settings.BlurSamples;
	existingConfig["Radial Blur"]["Blur Direction"] =
		glm::vec2(m_Settings.BlurDirection.x, m_Settings.BlurDirection.y);
	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void RadialBlurPass::Load()
{
	YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
	m_Settings.BlurStrength = config["Radial Blur"]["Blur Strength"]
		? config["Radial Blur"]["Blur Strength"].as<float>()
		: 0.0f;
	auto blurDirection = config["Radial Blur"]["Blur Direction"]
		? config["Radial Blur"]["Blur Direction"].as<glm::vec2>()
		: glm::vec2(0.0f);
	m_Settings.BlurDirection = glm::vec3(blurDirection.x, blurDirection.y, 0.0f);
	m_Settings.BlurSamples =
		config["Radial Blur"]["Blur Samples"] ? config["Radial Blur"]["Blur Samples"].as<int>() : 10;
}

void RadialBlurPass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;

	ImagePass::OnResize(targetSize);
	m_RadialBlurOutput->Resize(targetSize.x, targetSize.y);
}
