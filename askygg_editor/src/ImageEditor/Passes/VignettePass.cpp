#include "VignettePass.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <utility>

VignettePass::VignettePass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void VignettePass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("Vignette");
	const auto& Window = askygg::Application::GetWindow();

	askygg::Texture2DSpecification VignetteTextureSpec = {
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

	VignetteTextureSpec.Name = m_OutputName;
	m_VignetteOutput = askygg::CreateRef<askygg::Texture2D>(VignetteTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_VignetteOutput);
}

void VignettePass::Submit(uint32_t textureID)
{
	m_Shader->Bind();
	askygg::Texture2D::BindTextureIDToSamplerSlot(0, textureID);
	m_Shader->UploadUniformInt("u_Texture", 0);
	m_Shader->UploadUniformFloat("u_Radius", m_Settings.Radius);
	m_Shader->UploadUniformFloat("u_Softness", m_Settings.Softness);

	glm::vec2 textureSize = { m_VignetteOutput->GetWidth(), m_VignetteOutput->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	m_VignetteOutput->BindToImageSlot(0, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void VignettePass::DrawUI()
{
	if (ImGui::CollapsingHeader("Vignette Settings"))
	{
		askygg::UI::UIFloat::Draw("Vignette Radius", &m_Settings.Radius);
		askygg::UI::UIFloat::Draw("Vignette Softness", &m_Settings.Softness);

		if (ImGui::Button("Save Vignette Settings"))
			Save();
	}
}

void VignettePass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["Vignette"]["Radius"] = m_Settings.Radius;
	existingConfig["Vignette"]["Softness"] = m_Settings.Softness;

	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void VignettePass::Load()
{
	YAML::Node node = YAML::LoadFile(m_SettingsFilePath);
	if (node["Vignette"])
	{
		m_Settings.Radius = node["Vignette"]["Radius"].as<float>();
		m_Settings.Softness = node["Vignette"]["Softness"].as<float>();
	}
}

void VignettePass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;

	ImagePass::OnResize(targetSize);
	m_VignetteOutput->Resize(targetSize.x, targetSize.y);
}
