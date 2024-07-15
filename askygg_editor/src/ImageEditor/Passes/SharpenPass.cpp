#include "SharpenPass.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <utility>

SharpenPass::SharpenPass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void SharpenPass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("Sharpen");
	const auto& Window = askygg::Application::GetWindow();

	askygg::Texture2DSpecification SharpenTextureSpec = {
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

	SharpenTextureSpec.Name = m_OutputName;
	m_SharpenOutput = askygg::CreateRef<askygg::Texture2D>(SharpenTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_SharpenOutput);
}

void SharpenPass::Submit(uint32_t textureID)
{
	m_Shader->Bind();
	askygg::Texture2D::BindTextureIDToSamplerSlot(0, textureID);
	m_Shader->UploadUniformInt("u_Texture", 0);
	m_Shader->UploadUniformFloat("u_SharpenStrength", m_Settings.SharpenStrength);

	glm::vec2 textureSize = { m_SharpenOutput->GetWidth(), m_SharpenOutput->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	m_SharpenOutput->BindToImageSlot(0, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void SharpenPass::DrawUI()
{
	if (ImGui::CollapsingHeader("Sharpen Settings"))
	{
		askygg::UI::UIFloat::Draw("Sharpen Strength", &m_Settings.SharpenStrength);
		if (ImGui::Button("Save Sharpen Settings"))
			Save();
	}
}

void SharpenPass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["Sharpen"]["Sharpen Strength"] = m_Settings.SharpenStrength;
	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void SharpenPass::Load()
{
	YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
	m_Settings.SharpenStrength = config["Sharpen"]["Sharpen Strength"]
		? config["Sharpen"]["Sharpen Strength"].as<float>()
		: 0.0f;
}

void SharpenPass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;

	ImagePass::OnResize(targetSize);
	m_SharpenOutput->Resize(targetSize.x, targetSize.y);
}
