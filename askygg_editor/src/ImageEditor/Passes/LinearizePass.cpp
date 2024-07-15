#include "LinearizePass.h"
#include "askygg/core/Application.h"
#include <utility>

LinearizePass::LinearizePass(std::string settingsFilePath)
	: ImagePass(std::move(settingsFilePath)) {}

void LinearizePass::Initialize()
{
	m_Shader = askygg::ShaderLibrary::Get("Linearize");
	const auto& Window = askygg::Application::GetWindow();

	askygg::Texture2DSpecification LinearTextureSpec = {
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

	LinearTextureSpec.Name = m_OutputName;
	m_Output = askygg::CreateRef<askygg::Texture2D>(LinearTextureSpec);
	askygg::TextureLibrary::AddTexture2D(m_Output);
}

void LinearizePass::Submit(uint32_t textureID)
{
	m_Shader->Bind();

	glm::vec2 textureSize = { m_Output->GetWidth(), m_Output->GetHeight() };
	auto	  workGroupsX = (uint32_t)glm::ceil((float)textureSize.x / (float)m_WorkGroupSize);
	auto	  workGroupsY = (uint32_t)glm::ceil((float)textureSize.y / (float)m_WorkGroupSize);

	askygg::Texture2D::BindTextureIDToImageSlot(textureID, 0, 0,
		askygg::ImageUtils::TextureAccessLevel::ReadOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA8);
	m_Output->BindToImageSlot(1, 0, askygg::ImageUtils::TextureAccessLevel::WriteOnly,
		askygg::ImageUtils::TextureShaderDataFormat::RGBA32F);

	m_Shader->DispatchCompute(workGroupsX, workGroupsY, 1);
	m_Shader->EnableShaderImageAccessBarrierBit();

	m_Shader->Unbind();
}

void LinearizePass::OnResize(const glm::vec2& targetSize)
{
	if (m_OutputSize == targetSize)
		return;

	ImagePass::OnResize(targetSize);
	m_Output->Resize(targetSize.x, targetSize.y);
}
