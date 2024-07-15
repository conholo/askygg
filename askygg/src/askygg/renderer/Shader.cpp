#include "askygg/core/Assert.h"
#include "askygg/renderer/Shader.h"
#include "PlatformRenderAPI.h"
#include "askygg/platform/renderer_platform/opengl/OpenGLShader.h"
#include "askygg/platform/PlatformPath.h"
#include <sstream>

namespace askygg
{
	Ref<Shader> Shader::Create(const std::string& shaderSourceFile)
	{
		switch (PlatformRenderAPI::GetPlatformRendererType())
		{
			case PlatformRenderAPI::API::None:
				YGG_ASSERT(false, "RendererAPI::None is currently not supported!");
			case PlatformRenderAPI::API::Vulkan:
				YGG_ASSERT(false, "RendererAPI::Vulkan is currently not supported!");
			case PlatformRenderAPI::API::Metal:
#ifdef E_PLATFORM_MACOS
				return CreateRef<MetalShader>(shaderSourceFile);
#endif
			case PlatformRenderAPI::API::OpenGL:
				return CreateRef<OpenGLShader>(shaderSourceFile);
		}
		YGG_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	ShaderUniform::ShaderUniform(std::string name, GLint location, ShaderAttributeType type,
		uint32_t size, uint32_t count, int32_t blockOffset)
		: m_Name(std::move(name)), m_Location(location), m_Type(type), m_Count(count), m_Size(size), m_BlockOffset(blockOffset) {}

	ShaderBlock::ShaderBlock(std::string name, uint32_t size, uint32_t memberCount, uint32_t binding,
		uint32_t blockIndex)
		: m_Name(std::move(name)), m_BlockSize(size), m_MemberCount(memberCount), m_Binding(binding), m_BlockIndex(blockIndex)
	{
		m_UBO = CreateRef<UniformBuffer>(size, binding);
	}

	void Shader::LogShaderData()
	{
		if (m_ActiveTotalUniformCount <= 0)
		{
			YGG_LOG_INFO("{} Shader has no active uniforms.", m_Name);
			return;
		}

		YGG_LOG_INFO("-------------------------------------------------");
		YGG_LOG_INFO("Shader data for: {}", m_Name);
		YGG_LOG_INFO("-------------------------------------------------\n");
		YGG_LOG_INFO("Active uniform count: {}", m_ActiveTotalUniformCount);
		YGG_LOG_INFO("Base block uniforms: {}", m_DefaultBlockUniformCount);
		YGG_LOG_INFO("Named blocks: {}", m_Blocks.size());
		YGG_LOG_INFO("Named block uniforms: {}", m_NamedBlockUniformCount);
		YGG_LOG_INFO("\n");
		YGG_LOG_INFO("-------------------------------------------------");
		YGG_LOG_INFO("Base Block uniforms:");
		YGG_LOG_INFO("-------------------------------------------------");
		for (auto [name, uniform] : m_BaseBlockUniforms)
		{
			YGG_LOG_INFO("-------------------------------------------------");
			YGG_LOG_INFO("Uniform Name: {}", name);
			YGG_LOG_INFO("		Size: {}", uniform.GetSize());
			YGG_LOG_INFO("		Type: {}", ShaderAttributeTypeToString[uniform.GetType()]);
			YGG_LOG_INFO("		BufferOffset: {}", uniform.GetBufferOffset());
			YGG_LOG_INFO("		Location: {}", uniform.GetLocation());
			YGG_LOG_INFO("-------------------------------------------------");
		}

		YGG_LOG_INFO("\n");
		YGG_LOG_INFO("-------------------------------------------------");
		YGG_LOG_INFO("Named Blocks:");
		YGG_LOG_INFO("-------------------------------------------------");
		for (auto [blockName, block] : m_Blocks)
		{
			YGG_LOG_INFO("Block Name: {}", blockName);
			YGG_LOG_INFO("Block Binding: {}", block.GetBinding());
			YGG_LOG_INFO("Block Size: {}", block.GetBlockSize());
			YGG_LOG_INFO("Member Count: {}", block.GetMemberCount());

			for (auto [Name, uniform] : block.GetUniforms())
			{
				YGG_LOG_INFO("-------------------------------------------------");
				YGG_LOG_INFO("		Uniform Name: {}", uniform.GetName());
				YGG_LOG_INFO("			Size: {}", uniform.GetSize());
				YGG_LOG_INFO("			Type: {}", ShaderAttributeTypeToString[uniform.GetType()]);
				YGG_LOG_INFO("			BlockOffset: {}", uniform.GetBlockOffset());
				YGG_LOG_INFO("-------------------------------------------------");
			}

			YGG_LOG_INFO("-------------------------------------------------\n");
		}
	}

	const std::unordered_map<std::string, ShaderUniform>&
	Shader::GetAllUniformsFromBlock(const std::string& BlockName)
	{
		YGG_ASSERT(m_Blocks.find(BlockName) != m_Blocks.end(),
			"Unable to retrieve uniforms with unknown block name: {}", BlockName);
		return m_Blocks[BlockName].GetUniforms();
	}

	std::vector<ShaderUniform> Shader::GetBaseBlockUniformsOfType(ShaderAttributeType Type)
	{
		std::vector<ShaderUniform> Match;
		for (auto [UniformName, Uniform] : m_BaseBlockUniforms)
		{
			if (Uniform.GetType() == Type)
				Match.push_back(Uniform);
		}

		return Match;
	}

	std::unordered_map<std::string, Ref<Shader>> ShaderLibrary::s_ShaderLibrary;

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		if (s_ShaderLibrary.find(shader->GetName()) == s_ShaderLibrary.end())
		{
			s_ShaderLibrary[shader->GetName()] = shader;
		}
		else
		{
			YGG_LOG_TRACE(
				"Shader already contained in Shader Library.  Attempted to add '{}' Shader to Library.",
				shader->GetName());
		}
	}

	void ShaderLibrary::Load(const std::string& filePath)
	{
		std::string fullPath = GetExecutablePath() + "/" + filePath;
		Ref<Shader> shader = Shader::Create(fullPath);
		Add(shader);
	}

	const Ref<Shader>& ShaderLibrary::Get(const std::string& name)
	{
		YGG_ASSERT(s_ShaderLibrary.find(name) != s_ShaderLibrary.end(),
			"No shader with name: '{}' found in Shader Library.", name);
		return s_ShaderLibrary.at(name);
	}

	void ShaderLibrary::Shutdown()
	{
		s_ShaderLibrary.clear();
	}

	bool ShaderLibrary::Has(const std::string& shaderName)
	{
		return s_ShaderLibrary.find(shaderName) != s_ShaderLibrary.end();
	}

	bool ShaderLibrary::IsEmpty()
	{
		return s_ShaderLibrary.empty();
	}
} // namespace askygg
