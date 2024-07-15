#include "ChromaticAberrationPass.h"

#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <utility>

ChromaticAberrationPass::ChromaticAberrationPass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void ChromaticAberrationPass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("ChromaticAberration");
	const auto&					   Window = askygg::Application::GetWindow();
	askygg::Texture2DSpecification ChromaticAberrationTextureSpec = {
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

	ChromaticAberrationTextureSpec.Name = m_OutputName;
	m_ChromaticAberrationOutput =
		askygg::CreateRef<askygg::Texture2D>(ChromaticAberrationTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_ChromaticAberrationOutput);
}

void ChromaticAberrationPass::Submit(uint32_t textureID)
{
	m_Shader->Bind();
	askygg::Texture2D::BindTextureIDToSamplerSlot(0, textureID);
	m_Shader->UploadUniformInt("u_Texture", 0);
	m_Shader->UploadUniformFloat("u_Strength", m_Settings.Strength);

	glm::vec2 textureSize = { m_ChromaticAberrationOutput->GetWidth(),
		m_ChromaticAberrationOutput->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	m_ChromaticAberrationOutput->BindToImageSlot(
		0, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void ChromaticAberrationPass::DrawUI()
{
	if (ImGui::CollapsingHeader("Chromatic Aberration Settings"))
	{
		askygg::UI::UIFloat::Draw("Aberration Strength", &m_Settings.Strength);
		if (ImGui::Button("Save Chromatic Aberration Settings"))
			Save();
	}
}

void ChromaticAberrationPass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["Chromatic Aberration"]["Aberration Strength"] = m_Settings.Strength;
	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void ChromaticAberrationPass::Load()
{
	YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
	m_Settings.Strength = config["Chromatic Aberration"]["Aberration Strength"]
		? config["Chromatic Aberration"]["Aberration Strength"].as<float>()
		: 0.0f;
}

void ChromaticAberrationPass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;
	ImagePass::OnResize(targetSize);
	m_ChromaticAberrationOutput->Resize(targetSize.x, targetSize.y);
}
