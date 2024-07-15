#pragma once

#include "glm/glm.hpp"
#include "askygg/core/Memory.h"
#include "askygg/renderer/BufferLayout.h"

namespace askygg
{
	class OpenGLShader : public Shader
	{
	public:
		explicit OpenGLShader(const std::string& filePath);
		~OpenGLShader() override;

		void Bind() const override;
		void Unbind() const override;

		int UploadUniformFloat(const std::string& name, float value) override;
		int UploadUniformFloat2(const std::string& name, const glm::vec2& value) override;
		int UploadUniformFloat3(const std::string& name, const glm::vec3& value) override;
		int UploadUniformFloat4(const std::string& name, const glm::vec4& value) override;
		int UploadUniformBool(const std::string& name, bool value) override;
		int UploadUniformInt(const std::string& name, int value) override;
		int UploadUniformIntArray(const std::string& name, uint32_t count, int* basePtr) override;
		int UploadUniformMat3(const std::string& name, const glm::mat3& matrix) override;
		int UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;
		int UploadUniformFloat2Array(const std::string& name, uint32_t count, glm::vec2* value) override;
		int UploadUniformFloat3Array(const std::string& name, uint32_t count, glm::vec3* value) override;

		void* GetUniformData(ShaderAttributeType type, GLint location) override;
		void  DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ) override;

		void EnableShaderImageAccessBarrierBit() override;

	private:
		static std::string							   ReadFile(const std::string& filePath);
		static std::unordered_map<GLenum, std::string> PreProcess(const std::string& source);
		void										   Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
		void										   Reflect();
	};
} // namespace askygg
