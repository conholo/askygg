#include "HSVAdjustmentPass.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include <utility>

HSVAdjustmentPass::HSVAdjustmentPass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void HSVAdjustmentPass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("HSVAdjust");
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
	m_HSVOutput = askygg::CreateRef<askygg::Texture2D>(SharpenTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_HSVOutput);
}

void HSVAdjustmentPass::Submit(uint32_t textureID)
{
	m_Shader->Bind();
	askygg::Texture2D::BindTextureIDToSamplerSlot(0, textureID);
	m_Shader->UploadUniformInt("u_Texture", 0);

	m_Settings.HueShift = std::clamp(m_Settings.HueShift, -1.0f, 1.0f);
	m_Settings.SaturationBoost = std::clamp(m_Settings.SaturationBoost, -1.0f, 1.0f);
	m_Settings.ValueBoost = std::clamp(m_Settings.ValueBoost, -1.0f, 1.0f);

	m_Shader->UploadUniformFloat("u_HueShift", m_Settings.HueShift);
	m_Shader->UploadUniformFloat("u_SaturationBoost", m_Settings.SaturationBoost);
	m_Shader->UploadUniformFloat("u_ValueBoost", m_Settings.ValueBoost);

	glm::vec2 textureSize = { m_HSVOutput->GetWidth(), m_HSVOutput->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	m_HSVOutput->BindToImageSlot(0, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void HSVAdjustmentPass::DrawUI()
{
	if (ImGui::CollapsingHeader("Hue Shift Settings"))
	{
		askygg::UI::UIFloat::Draw("Hue Shift Amount", &m_Settings.HueShift);
		askygg::UI::UIFloat::Draw("Saturation Amount", &m_Settings.SaturationBoost);
		askygg::UI::UIFloat::Draw("Value Amount", &m_Settings.ValueBoost);
		if (ImGui::Button("Save Hue Shift Settings"))
			Save();
	}
}

void HSVAdjustmentPass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["Hue Shift"]["Hue Shift Amount"] = m_Settings.HueShift;
	existingConfig["Hue Shift"]["Saturation Boost Amount"] = m_Settings.SaturationBoost;
	existingConfig["Hue Shift"]["Value Boost Amount"] = m_Settings.ValueBoost;
	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void HSVAdjustmentPass::Load()
{
	YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
	m_Settings.HueShift = config["Hue Shift"]["Hue Shift Amount"]
		? config["Hue Shift"]["Hue Shift Amount"].as<float>()
		: 0.0f;
	m_Settings.SaturationBoost = config["Hue Shift"]["Saturation Boost Amount"]
		? config["Hue Shift"]["Saturation Boost Amount"].as<float>()
		: 0.0f;
	m_Settings.ValueBoost = config["Hue Shift"]["Value Boost Amount"]
		? config["Hue Shift"]["Value Boost Amount"].as<float>()
		: 0.0f;
}

void HSVAdjustmentPass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;
	ImagePass::OnResize(targetSize);
	m_HSVOutput->Resize(targetSize.x, targetSize.y);
}
