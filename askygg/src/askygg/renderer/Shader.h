#pragma once

#include "glm/glm.hpp"
#include "askygg/core/Memory.h"
#include "askygg/renderer/BufferLayout.h"
#include "askygg/renderer/UniformBuffer.h"

namespace askygg
{
	typedef unsigned int GLenum;
	typedef int			 GLint;

	enum class ShaderUniformType
	{
		None = 0,
		Float,
		RangeFloat,
		Vec2,
		Vec3,
		Vec4,
		Color,
		Texture,
		Int,
		Bool
	};

	class ShaderUniform
	{
	public:
		ShaderUniform() = default;
		ShaderUniform(std::string name, GLint location, ShaderAttributeType type, uint32_t size,
			uint32_t count, int32_t offset);

		const std::string&	GetName() const { return m_Name; }
		GLint				GetLocation() const { return m_Location; }
		ShaderAttributeType GetType() const { return m_Type; }
		uint32_t			GetCount() const { return m_Count; }
		uint32_t			GetSize() const { return m_Size; }
		int32_t				GetBlockOffset() const { return m_BlockOffset; }
		uint32_t			GetBufferOffset() const { return m_BufferOffset; }
		void				SetBufferOffsetForDefaultBlockUniform(uint32_t offset) { m_BufferOffset = offset; }

	private:
		std::string			m_Name;
		GLint				m_Location = -1;
		ShaderAttributeType m_Type = ShaderAttributeType::None;
		uint32_t			m_Count = 0;
		uint32_t			m_Size = 0;
		int32_t				m_BlockOffset = 0;
		uint32_t			m_BufferOffset = 0;
	};

	class ShaderBlock
	{
	public:
		ShaderBlock() = default;
		ShaderBlock(std::string name, uint32_t size, uint32_t memberCount, uint32_t binding,
			uint32_t blockIndex);

		[[nodiscard]] const std::unordered_map<std::string, ShaderUniform>& GetUniforms() const
		{
			return m_Uniforms;
		}
		void AddUniform(const ShaderUniform& Uniform) { m_Uniforms[Uniform.GetName()] = Uniform; }

		[[nodiscard]] const std::string& GetName() const { return m_Name; }
		[[nodiscard]] uint32_t			 GetBlockSize() const { return m_BlockSize; }
		[[nodiscard]] uint32_t			 GetMemberCount() const { return m_MemberCount; }
		[[nodiscard]] uint32_t			 GetBinding() const { return m_Binding; }
		[[nodiscard]] uint32_t			 GetBlockIndex() const { return m_BlockIndex; }
		bool							 Has(const std::string& UniformName)
		{
			return m_Uniforms.find(UniformName) != m_Uniforms.end();
		}
		ShaderUniform* Get(const std::string& UniformName)
		{
			return Has(UniformName) ? &m_Uniforms[UniformName] : nullptr;
		}

	private:
		Ref<UniformBuffer>							   m_UBO;
		std::unordered_map<std::string, ShaderUniform> m_Uniforms;
		std::string									   m_Name;
		uint32_t									   m_BlockSize = 0;
		uint32_t									   m_MemberCount = 0;
		uint32_t									   m_Binding = 0;
		uint32_t									   m_BlockIndex = 0;
	};

	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual int UploadUniformFloat(const std::string& name, float value) = 0;
		virtual int UploadUniformFloat2(const std::string& name, const glm::vec2& value) = 0;
		virtual int UploadUniformFloat3(const std::string& name, const glm::vec3& value) = 0;
		virtual int UploadUniformFloat4(const std::string& name, const glm::vec4& value) = 0;

		virtual int	  UploadUniformBool(const std::string& name, bool value) = 0;
		virtual int	  UploadUniformInt(const std::string& name, int value) = 0;
		virtual int	  UploadUniformIntArray(const std::string& name, uint32_t count, int* basePtr) = 0;
		virtual int	  UploadUniformMat3(const std::string& name, const glm::mat3& matrix) = 0;
		virtual int	  UploadUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;
		virtual int	  UploadUniformFloat2Array(const std::string& name, uint32_t count,
			  glm::vec2* value) = 0;
		virtual int	  UploadUniformFloat3Array(const std::string& name, uint32_t count,
			  glm::vec3* value) = 0;
		virtual void* GetUniformData(ShaderAttributeType type, GLint location) = 0;
		virtual void  DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ) = 0;
		virtual void  EnableShaderImageAccessBarrierBit() = 0;

	public:
		uint32_t	GetID() const { return m_ID; }
		std::string GetName() const { return m_Name; }

		void												  LogShaderData();
		const ShaderUniform&								  GetUniform(const std::string& name) { return m_BaseBlockUniforms[name]; }
		const std::unordered_map<std::string, ShaderUniform>& GetBaseBlockUniforms() const
		{
			return m_BaseBlockUniforms;
		}
		const std::unordered_map<std::string, ShaderUniform>&
													 GetAllUniformsFromBlock(const std::string& BlockName);
		std::unordered_map<std::string, ShaderBlock> GetNamedBlocks() { return m_Blocks; }
		std::vector<ShaderUniform>					 GetBaseBlockUniformsOfType(ShaderAttributeType Type);

		static Ref<Shader> Create(const std::string& shaderSourceFile);

	protected:
		std::unordered_map<std::string, ShaderUniform> m_BaseBlockUniforms;
		std::unordered_map<std::string, ShaderBlock>   m_Blocks;
		uint32_t									   m_ActiveTotalUniformCount = 0;
		uint32_t									   m_NamedBlockUniformCount = 0;
		uint32_t									   m_DefaultBlockUniformCount = 0;
		std::string									   m_Name;
		uint32_t									   m_ID;
		bool										   m_IsCompute = false;
	};

	class ShaderLibrary
	{
	public:
		static void Add(const Ref<Shader>& shader);
		static void Load(const std::string& filePath);

		static bool				  Has(const std::string& shaderName);
		static const Ref<Shader>& Get(const std::string& name);
		static void				  Shutdown();

		static bool IsEmpty();

	private:
		static std::unordered_map<std::string, Ref<Shader>> s_ShaderLibrary;
	};
} // namespace askygg
