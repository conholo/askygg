#include "BarrelDistortionPass.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <utility>

BarrelDistortionPass::BarrelDistortionPass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void BarrelDistortionPass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("BarrelDistortion");
	const auto& Window = askygg::Application::GetWindow();

	askygg::Texture2DSpecification BarrelDistortionTextureSpec = {
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

	BarrelDistortionTextureSpec.Name = m_OutputName;
	m_BarrelDistortionOutput = askygg::CreateRef<askygg::Texture2D>(BarrelDistortionTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_BarrelDistortionOutput);
}

void BarrelDistortionPass::Submit(uint32_t textureID)
{
	m_Shader->Bind();
	askygg::Texture2D::BindTextureIDToSamplerSlot(0, textureID);
	m_Shader->UploadUniformInt("u_Texture", 0);
	m_Shader->UploadUniformFloat2("u_Distortion", m_Settings.DistortionStrength);

	glm::vec2 textureSize = { m_BarrelDistortionOutput->GetWidth(), m_BarrelDistortionOutput->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	m_BarrelDistortionOutput->BindToImageSlot(0, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void BarrelDistortionPass::DrawUI()
{
	if (ImGui::CollapsingHeader("Barrel Distortion Settings"))
	{
		askygg::UI::UIVector3::Draw("Barrel Distortion Strength", &m_Settings.DistortionStrength);
		if (ImGui::Button("Save Barrel Distortion Settings"))
			Save();
	}
}

void BarrelDistortionPass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["Barrel Distortion"]["Distortion Strength"] =
		glm::vec2(m_Settings.DistortionStrength.x, m_Settings.DistortionStrength.y);
	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void BarrelDistortionPass::Load()
{
	YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
	glm::vec2  distortionStrength =
		 config["Barrel Distortion"]["Distortion Strength"]
		 ? config["Barrel Distortion"]["Distortion Strength"].as<glm::vec2>()
		 : glm::vec2(0.0f, 0.0f);
	m_Settings.DistortionStrength = glm::vec3(distortionStrength.x, distortionStrength.y, 0.0f);
}

void BarrelDistortionPass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;

	ImagePass::OnResize(targetSize);
	m_BarrelDistortionOutput->Resize(targetSize.x, targetSize.y);
}
