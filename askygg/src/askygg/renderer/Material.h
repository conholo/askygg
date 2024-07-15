#pragma once
#include "askygg/renderer/Shader.h"
#include "askygg/core/Buffer.h"
#include "askygg/core/Memory.h"
#include "askygg/core/Log.h"

namespace askygg
{
	class Texture2D;
	enum class RenderFlag;

#define LOG_MATERIAL 0

	const std::string EngineShaderName = "EnginePBR";
	struct TextureUniform
	{
		uint32_t RendererID = 0;
		int32_t	 TextureUnit = -1;
		int32_t	 HideInUI = 0;
	};

	struct MaterialUniformData
	{
		std::unordered_map<std::string, float>			FloatUniforms;
		std::unordered_map<std::string, int>			IntUniforms;
		std::unordered_map<std::string, TextureUniform> TextureUniforms;
		std::unordered_map<std::string, glm::vec2>		Vec2Uniforms;
		std::unordered_map<std::string, glm::vec3>		Vec3Uniforms;
		std::unordered_map<std::string, glm::vec4>		Vec4Uniforms;
	};

	class Material
	{
	public:
		Material() = default;
		Material(std::string name, const Ref<Shader>& shader);
		Ref<Material> Clone(const std::string& cloneName);

		Ref<Shader>		   GetShader() const { return m_Shader; }
		const std::string& GetName() const { return m_Name; }

		void UploadStagedUniforms();
		void BindSamplerTexturesToRenderContext();

		template <typename T>
		void Set(const std::string& name, const T& data)
		{
			const auto* uniform = FindBaseBlockShaderUniform(name);

			if (uniform == nullptr)
				return;
			m_BaseBlockStorageBuffer.Write<T>((uint8_t*)&data, uniform->GetSize(),
				uniform->GetBufferOffset());
		}

		template <typename T>
		T* Get(const std::string& name)
		{
			const auto* uniform = FindBaseBlockShaderUniform(name);

			if (uniform == nullptr)
				return nullptr;

			return m_BaseBlockStorageBuffer.Read<T>(uniform->GetBufferOffset());
		}

		bool Has(const std::string& name) const
		{
			const auto* uniform = FindBaseBlockShaderUniform(name);
			return uniform != nullptr;
		}

		const ShaderUniform* FindBaseBlockShaderUniform(const std::string& name) const;

		MaterialUniformData GetMaterialUniformData();
		void				Bind() const { m_Shader->Bind(); }
		void				Unbind() const { m_Shader->Unbind(); }

	private:
		void AllocateBaseBlockStorageBuffer();
		void InitializeBaseBlockStorageBufferWithUniformDefaults() const;

		Ref<Shader>								m_Shader;
		Buffer									m_BaseBlockStorageBuffer;
		std::unordered_map<std::string, Buffer> m_NamedBlockStorageBuffers;
		std::string								m_Name;
	};
} // namespace askygg
