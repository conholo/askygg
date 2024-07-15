#include "MultiPassBloomPass.h"
#include "askygg/core/Application.h"
#include "askygg/ui/PropertyDrawer.h"
#include "askygg/platform/PlatformPath.h"
#include <imgui.h>
#include <yaml-cpp/yaml.h>
#include <fstream>

MultiPassBloomPass::MultiPassBloomPass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void MultiPassBloomPass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("MultiPassBloom");

	const glm::vec2 WindowDimensions = { askygg::Application::GetWindow().GetWidth(),
		askygg::Application::GetWindow().GetHeight() };
	uint32_t		halfWidth = m_OutputSize.x / 2;
	uint32_t		halfHeight = m_OutputSize.y / 2;
	halfWidth += m_WorkGroupSize - halfWidth % m_WorkGroupSize;
	halfHeight += m_WorkGroupSize - halfHeight % m_WorkGroupSize;

	askygg::Texture2DSpecification BloomTextureSpecification = {
		askygg::ImageUtils::WrapMode::ClampToEdge,
		askygg::ImageUtils::WrapMode::ClampToEdge,
		askygg::ImageUtils::FilterMode::LinearMipLinear,
		askygg::ImageUtils::FilterMode::Linear,
		askygg::ImageUtils::ImageInternalFormat::RGBA32F,
		askygg::ImageUtils::ImageDataLayout::RGBA,
		askygg::ImageUtils::ImageDataType::Float,
		halfWidth,
		halfHeight
	};

	m_BloomComputeTextures.resize(3);
	BloomTextureSpecification.Name = "Bloom 1";
	m_BloomComputeTextures[0] = askygg::CreateRef<askygg::Texture2D>(BloomTextureSpecification);
	BloomTextureSpecification.Name = "Bloom 2";
	m_BloomComputeTextures[1] = askygg::CreateRef<askygg::Texture2D>(BloomTextureSpecification);
	BloomTextureSpecification.Name = m_OutputName;
	m_BloomComputeTextures[2] = askygg::CreateRef<askygg::Texture2D>(BloomTextureSpecification);

	askygg::TextureLibrary::AddTexture2D(m_BloomComputeTextures[2]);
}

void MultiPassBloomPass::Submit(uint32_t textureID)
{
	m_Shader->Bind();

	struct BloomConstants
	{
		glm::vec4 Params{};
		float	  LOD = 0.0f;
		int		  Mode = 0;
	} bloomConstants;

	bloomConstants.Params = { m_Settings.BloomThreshold,
		m_Settings.BloomThreshold - m_Settings.BloomKnee,
		m_Settings.BloomKnee * 2.0f, 0.25f / m_Settings.BloomKnee };

	//------------------ PREFILTER -----------------//
	uint32_t workGroupsX = m_BloomComputeTextures[0]->GetWidth() / m_WorkGroupSize;
	uint32_t workGroupsY = m_BloomComputeTextures[0]->GetHeight() / m_WorkGroupSize;

	{
		bloomConstants.Mode = 0;
		m_Shader->UploadUniformFloat4("u_Params", bloomConstants.Params);
		m_Shader->UploadUniformFloat("u_LOD", bloomConstants.LOD);
		m_Shader->UploadUniformInt("u_Mode", bloomConstants.Mode);
		askygg::Texture2D::BindTextureIDToSamplerSlot(0, textureID);
		m_Shader->UploadUniformInt("u_Texture", 0);
		m_BloomComputeTextures[0]->BindToImageSlot(
			0, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
			askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);

		m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
		m_Shader->EnableShaderImageAccessBarrierBit();
		askygg::Texture2D::ClearBinding();
	}
	//------------------ PREFILTER -----------------//

	//------------------ DOWNSAMPLE -----------------//
	bloomConstants.Mode = 1;
	uint32_t mips = m_BloomComputeTextures[0]->GetMipLevelCount() - 2;

	for (uint32_t mip = 1; mip < mips; mip++)
	{
		{
			auto [mipWidth, mipHeight] = m_BloomComputeTextures[0]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)m_WorkGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)m_WorkGroupSize);

			bloomConstants.LOD = mip - 1.0f;
			// Write to 1
			m_BloomComputeTextures[1]->BindToImageSlot(
				0, mip, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
				askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
			// Read from 0 (starts pre-filtered)
			m_BloomComputeTextures[0]->BindToSamplerSlot(0);
			m_Shader->UploadUniformInt("u_Texture", 0);
			m_Shader->UploadUniformInt("u_Mode", bloomConstants.Mode);
			m_Shader->UploadUniformFloat("u_LOD", bloomConstants.LOD);
			m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
			m_Shader->EnableShaderImageAccessBarrierBit();
		}

		{
			bloomConstants.LOD = mip;
			// Write to 0
			m_BloomComputeTextures[0]->BindToImageSlot(
				0, mip, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
				askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
			// Read from 1
			m_BloomComputeTextures[1]->BindToSamplerSlot(0);

			m_Shader->UploadUniformInt("u_Texture", 0);
			m_Shader->UploadUniformInt("u_Mode", bloomConstants.Mode);
			m_Shader->UploadUniformFloat("u_LOD", bloomConstants.LOD);
			m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
			m_Shader->EnableShaderImageAccessBarrierBit();
		}
	}
	//------------------ DOWNSAMPLE -----------------//

	//------------------ UPSAMPLE_FIRST -----------------//
	{
		bloomConstants.Mode = 2;
		bloomConstants.LOD--;
		// Write to 2 at smallest image in up-sampling mip chain
		m_BloomComputeTextures[2]->BindToImageSlot(
			0, mips - 2, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
			askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
		// Read from 0 (fully down-sampled)
		m_BloomComputeTextures[0]->BindToSamplerSlot(0);

		m_Shader->UploadUniformInt("u_Texture", 0);
		m_Shader->UploadUniformInt("u_Mode", bloomConstants.Mode);
		m_Shader->UploadUniformFloat("u_LOD", bloomConstants.LOD);
		m_Shader->UploadUniformFloat("u_Radius", m_Settings.Radius);
		m_Shader->UploadUniformFloat("u_UpsampleTightenFactor", m_Settings.UpsampleTightenFactor);

		auto [mipWidth, mipHeight] = m_BloomComputeTextures[2]->GetMipSize(mips - 2);
		workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)m_WorkGroupSize);
		workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)m_WorkGroupSize);
		m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
		m_Shader->EnableShaderImageAccessBarrierBit();
	}
	//------------------ UPSAMPLE_FIRST -----------------//

	//------------------ UPSAMPLE -----------------//
	{
		bloomConstants.Mode = 3;
		for (int32_t mip = mips - 3; mip >= 0; mip--)
		{
			auto [mipWidth, mipHeight] = m_BloomComputeTextures[2]->GetMipSize(mip);
			workGroupsX = (uint32_t)glm::ceil((float)mipWidth / (float)m_WorkGroupSize);
			workGroupsY = (uint32_t)glm::ceil((float)mipHeight / (float)m_WorkGroupSize);

			bloomConstants.LOD = mip;

			// Write to 2
			m_Shader->EnableShaderImageAccessBarrierBit();
			m_BloomComputeTextures[2]->BindToImageSlot(
				0, mip, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
				askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);
			// Read from 0
			m_BloomComputeTextures[0]->BindToSamplerSlot(0);
			m_Shader->UploadUniformInt("u_Texture", 0);
			m_BloomComputeTextures[2]->BindToSamplerSlot(1);
			m_Shader->UploadUniformInt("u_BloomTexture", 1);
			m_Shader->UploadUniformInt("u_Mode", bloomConstants.Mode);
			m_Shader->UploadUniformFloat("u_LOD", bloomConstants.LOD);
            m_Shader->UploadUniformFloat("u_Radius", m_Settings.Radius);
            m_Shader->UploadUniformFloat("u_UpsampleTightenFactor", m_Settings.UpsampleTightenFactor);
			m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
			m_Shader->EnableShaderImageAccessBarrierBit();
		}
	}
	//------------------ UPSAMPLE -----------------//

	askygg::Texture2D::ClearBinding();
	m_Shader->Unbind();
}

void MultiPassBloomPass::DrawUI()
{
	if (ImGui::CollapsingHeader("Multi-Pass Bloom Settings"))
	{
        askygg::UI::UIFloat::Draw("Threshold", &m_Settings.BloomThreshold);
        askygg::UI::UIFloat::Draw("Knee", &m_Settings.BloomKnee);
        askygg::UI::UIFloat::Draw("Radius", &m_Settings.Radius);
        askygg::UI::UIFloat::Draw("Tighten Factor", &m_Settings.UpsampleTightenFactor);
        askygg::UI::UIBool::Draw("Display Compute Textures", &m_Settings.DisplayBloomDebug);

        if (m_Settings.DisplayBloomDebug)
        {
            float aspect = (float)m_OutputSize.x / (float)m_OutputSize.y;
            ImGui::Image(reinterpret_cast<ImTextureID>(m_BloomComputeTextures[0]->GetID()),
                { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
            ImGui::Image(reinterpret_cast<ImTextureID>(m_BloomComputeTextures[1]->GetID()),
                { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
            ImGui::Image(reinterpret_cast<ImTextureID>(m_BloomComputeTextures[2]->GetID()),
                { 300 * aspect, 300 }, { 0, 1 }, { 1, 0 });
        }

		if (ImGui::Button("Save Multi-Pass Bloom Settings"))
			Save();
	}
}

void MultiPassBloomPass::Save()
{
	YAML::Node existingConfig;
	existingConfig = YAML::LoadFile(m_SettingsFilePath);
	existingConfig["MultiPassBloom"]["Threshold"] = m_Settings.BloomThreshold;
	existingConfig["MultiPassBloom"]["Knee"] = m_Settings.BloomKnee;
	existingConfig["MultiPassBloom"]["Radius"] = m_Settings.Radius;
	existingConfig["MultiPassBloom"]["Upsample Tighten Factor"] = m_Settings.UpsampleTightenFactor;
	std::ofstream outFile(m_SettingsFilePath);
	outFile << existingConfig;
}

void MultiPassBloomPass::Load()
{
	YAML::Node config = YAML::LoadFile(m_SettingsFilePath);
	m_Settings.BloomThreshold = config["MultiPassBloom"]["Threshold"] ? config["MultiPassBloom"]["Threshold"].as<float>() : 2.0f;
	m_Settings.BloomKnee = config["MultiPassBloom"]["Knee"] ? config["MultiPassBloom"]["Knee"].as<float>() : 0.2f;
	m_Settings.Radius = config["MultiPassBloom"]["Radius"] ? config["MultiPassBloom"]["Radius"].as<float>() : 1.0f;
	m_Settings.UpsampleTightenFactor = config["MultiPassBloom"]["Upsample Tighten Factor"] ? config["MultiPassBloom"]["Upsample Tighten Factor"].as<float>() : 16.0f;
}

void MultiPassBloomPass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;

	ImagePass::OnResize(targetSize);
	uint32_t halfWidth = targetSize.x / 2;
	uint32_t halfHeight = targetSize.y / 2;
	halfWidth += m_WorkGroupSize - halfWidth % m_WorkGroupSize;
	halfHeight += m_WorkGroupSize - halfHeight % m_WorkGroupSize;
	m_BloomComputeTextures[0]->Resize(halfWidth, halfHeight);
	m_BloomComputeTextures[1]->Resize(halfWidth, halfHeight);
	m_BloomComputeTextures[2]->Resize(halfWidth, halfHeight);
}
