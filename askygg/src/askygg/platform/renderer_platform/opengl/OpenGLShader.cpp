#include "askygg/core/Assert.h"
#include "askygg/renderer/Material.h"

#include "askygg/platform/renderer_platform/opengl/OpenGLShader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>

namespace askygg
{
	static ShaderAttributeType ShaderDataTypeFromGLenum(GLenum value)
	{
		switch (value)
		{
			case GL_FLOAT:
				return ShaderAttributeType::Float;
			case GL_FLOAT_VEC2:
				return ShaderAttributeType::Float2;
			case GL_FLOAT_VEC3:
				return ShaderAttributeType::Float3;
			case GL_FLOAT_VEC4:
				return ShaderAttributeType::Float4;
			case GL_INT:
				return ShaderAttributeType::Int;
			case GL_FLOAT_MAT3:
				return ShaderAttributeType::Mat3;
			case GL_FLOAT_MAT4:
				return ShaderAttributeType::Mat4;
			case GL_SAMPLER_2D:
				return ShaderAttributeType::Sampler2D;
			case GL_SAMPLER_CUBE:
				return ShaderAttributeType::SamplerCube;
			case GL_SAMPLER_2D_ARRAY:
				return ShaderAttributeType::Sampler2DArray;
			default:
				return ShaderAttributeType::None;
		}
	}

	OpenGLShader::OpenGLShader(const std::string& filePath)
	{
		const size_t shaderLocationOffset = filePath.rfind('/') + 1;
		const size_t extensionOffset = filePath.find_first_of('.', shaderLocationOffset);

		m_Name = filePath.substr(shaderLocationOffset, extensionOffset - shaderLocationOffset);

		std::string source = ReadFile(filePath);
		const auto	shaderSources = PreProcess(source);
		Compile(shaderSources);
		Reflect();
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteShader(m_ID);
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_ID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	std::string OpenGLShader::ReadFile(const std::string& filePath)
	{
		std::string	  result;
		std::ifstream input(filePath, std::ios::binary | std::ios::in);
		YGG_ASSERT(input, "Unable to read shader source file: '{}'", filePath);

		input.seekg(0, std::ios::end);
		const size_t size = input.tellg();

		if (size != -1)
		{
			result.resize(size);
			input.seekg(0, std::ios::beg);
			input.read(result.data(), size);
		}

		return result;
	}

	GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment")
			return GL_FRAGMENT_SHADER;
		if (type == "compute")
			return GL_COMPUTE_SHADER;
		if (type == "geometry")
			return GL_GEOMETRY_SHADER;

		return GL_FALSE;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source)
	{
		const char*	 typeToken = "#type";
		const size_t typeTokenLength = strlen(typeToken);
		size_t		 position = source.find(typeToken, 0);

		std::unordered_map<GLenum, std::string> result;

		while (position != std::string::npos)
		{
			const size_t endOfLine = source.find_first_of("\r\n", position);
			const size_t beginShaderType = position + typeTokenLength + 1;
			std::string	 type = source.substr(beginShaderType, endOfLine - beginShaderType);

			const size_t nextLinePosition = source.find_first_not_of("\r\n", endOfLine);
			position = source.find(typeToken, nextLinePosition);
			result[ShaderTypeFromString(type)] =
				(position == std::string::npos)
				? source.substr(nextLinePosition)
				: source.substr(nextLinePosition, position - nextLinePosition);
		}

		return result;
	}

	void* OpenGLShader::GetUniformData(ShaderAttributeType type, GLint location)
	{
		switch (type)
		{
			case ShaderAttributeType::None:
			{
				return nullptr;
			}
			case ShaderAttributeType::Float:
			case ShaderAttributeType::Float2:
			case ShaderAttributeType::Float3:
			case ShaderAttributeType::Float4:
			{
				auto* data = (GLfloat*)malloc(ShaderDataTypeSize(type));
				glGetUniformfv(m_ID, location, data);

				return data;
			}
			case ShaderAttributeType::Sampler2DArray:
			case ShaderAttributeType::SamplerCube:
			case ShaderAttributeType::Sampler2D:
			{
				auto* data = (GLint*)malloc(ShaderDataTypeSize(type));
				glGetUniformiv(m_ID, location, data);
				(TextureUniform*)data;

				return data;
			}
			case ShaderAttributeType::Int:
			{
				auto* data = (GLint*)malloc(ShaderDataTypeSize(type));
				glGetUniformiv(m_ID, location, data);
				return data;
			}
			case ShaderAttributeType::Mat3:
			case ShaderAttributeType::Mat4:
			{
				YGG_LOG_INFO("Currently unable to retrieve uniform data for uniforms of type Mat3/Mat4");
				return nullptr;
			}
		}

		return nullptr;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		const GLuint		program = glCreateProgram();
		std::vector<GLuint> shaderIDs;

		for (auto& kv : shaderSources)
		{
			const GLenum shaderType = kv.first;
			m_IsCompute = shaderType == GL_COMPUTE_SHADER;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(shaderType);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
				std::string shaderLog(infoLog.begin(), infoLog.end());
				std::string shaderTypeName;

				if (shaderType == GL_FRAGMENT_SHADER)
					shaderTypeName = "FRAGMENT COMPILATION ERROR";
				else if (shaderType == GL_VERTEX_SHADER)
					shaderTypeName = "VERTEX COMPILATION ERROR";
				else if (shaderType == GL_GEOMETRY_SHADER)
					shaderTypeName = "GEOMETRY COMPILATION ERROR";
				else if (shaderType == GL_COMPUTE_SHADER)
					shaderTypeName = "COMPUTE COMPILATION ERROR";

				YGG_LOG_ERROR(m_Name);
				YGG_LOG_ERROR("Error Type: {}", shaderTypeName);
				YGG_LOG_ERROR("Error Log:\n{}", shaderLog);

				glDeleteShader(shader);

				break;
			}

			glAttachShader(program, shader);
			shaderIDs.push_back(shader);
		}

		m_ID = program;

		glLinkProgram(program);

		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			std::stringstream ss;

			const std::string messageHeader = m_Name + ": LINKING ERROR: ";
			ss << messageHeader << infoLog.data();
			YGG_LOG_ERROR(ss.str());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
				glDeleteShader(id);

			return;
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
	}

	void OpenGLShader::Reflect()
	{
		auto activeCount = 0;
		glGetProgramiv(m_ID, GL_ACTIVE_UNIFORMS, &activeCount);

		m_ActiveTotalUniformCount = activeCount;

		if (activeCount == 0)
			return;

		m_ActiveTotalUniformCount = activeCount;
		GLsizei maxUniformLength;
		glGetProgramiv(m_ID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLength);

		std::vector<std::string> activeUniformNames(activeCount);

		for (GLuint i = 0; i < m_ActiveTotalUniformCount; ++i)
		{
			GLsizei				length;
			std::vector<GLchar> nameBuffer(maxUniformLength);

			glGetActiveUniformName(m_ID, i, maxUniformLength, &length, nameBuffer.data());

			// Write to member vector to store names.
			std::string name(nameBuffer.data(), length);

			// Store name
			activeUniformNames[i] = name;
		}

		// Vector of buffers for each uniform name.
		std::vector<const char*> nameBuffer(activeCount);

		// Convert from string to const char* for glGetUniformIndices -- it needs a const char**.
		for (GLsizei i = 0; i < activeCount; i++)
			nameBuffer[i] = (activeUniformNames[i]).c_str();

		// Declare and allocate memory for a vector containing the uniform indices.
		std::vector<GLuint> indices(activeCount);
		// Get the uniform indices.
		glGetUniformIndices(m_ID, activeCount, nameBuffer.data(), indices.data());

		// Get some data about the uniform.
		std::vector<GLint> types(activeCount);
		std::vector<GLint> counts(activeCount);
		std::vector<GLint> blockIndices(activeCount);
		std::vector<GLint> blockOffsets(activeCount);
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_TYPE, types.data());
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_SIZE, counts.data());
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_BLOCK_INDEX,
			blockIndices.data());
		glGetActiveUniformsiv(m_ID, activeCount, indices.data(), GL_UNIFORM_OFFSET, blockOffsets.data());

		// Create ShaderUniforms/Blocks

		uint32_t currentOffset = 0;

		std::vector<uint32_t> registeredBlockIndices;

		// Named Blocks
		for (uint32_t i = 0; i < blockIndices.size(); i++)
		{
			// If the block index is -1 (base block) or we've already registered the block, move on.
			if (blockIndices[i] == -1 || std::find(registeredBlockIndices.begin(), registeredBlockIndices.end(), blockIndices[i]) != registeredBlockIndices.end())
				continue;

			// Otherwise, query OpenGL for data about the block and construct a new Block object to store
			// the data.
			GLint binding;
			GLint blockSize;
			GLint activeCount;

			std::vector<GLchar> nameBuffer(maxUniformLength);
			GLint				nameLength;

			glGetActiveUniformBlockName(m_ID, blockIndices[i], maxUniformLength, &nameLength,
				nameBuffer.data());
			glGetActiveUniformBlockiv(m_ID, blockIndices[i], GL_UNIFORM_BLOCK_BINDING, &binding);
			glGetActiveUniformBlockiv(m_ID, blockIndices[i], GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
			glGetActiveUniformBlockiv(m_ID, blockIndices[i], GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS,
				&activeCount);

			GLint* activeIndices = (GLint*)malloc(sizeof(GLint) * activeCount);
			glGetActiveUniformBlockiv(m_ID, blockIndices[i], GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES,
				activeIndices);

			std::string name(nameBuffer.data(), nameLength);

			m_Blocks[name] = ShaderBlock(name, blockSize, activeCount, binding, blockIndices[i]);

			registeredBlockIndices.push_back(blockIndices[i]);
		}

		m_NamedBlockUniformCount = 0;
		m_DefaultBlockUniformCount = 0;

		// Create uniform objects and store in respective data structures.
		// Named block uniforms -> m_Blocks unordered map
		// Default block uniforms -> m_BaseBlockUniforms unordered map
		for (uint32_t i = 0; i < m_ActiveTotalUniformCount; i++)
		{
			const std::string name = activeUniformNames[i];
			if (name.substr(0, 3) == "gl_")
				continue;
			GLenum				type = types[i];
			GLint				location = glGetUniformLocation(m_ID, name.c_str());
			ShaderAttributeType shaderDataType = ShaderDataTypeFromGLenum(type);
			GLint				blockOffset = blockOffsets[i];

			GLint	 count = counts[i];
			uint32_t size = count * ShaderDataTypeSize(shaderDataType);

			ShaderUniform uniform(name, location, shaderDataType, size, count, blockOffset);

			if (blockIndices[i] != -1)
			{
				// Add the uniform to it's respective block.

				std::vector<GLchar> nameBuffer(maxUniformLength);
				GLint				blockNameLength;

				glGetActiveUniformBlockName(m_ID, blockIndices[i], maxUniformLength, &blockNameLength,
					nameBuffer.data());

				std::string name(nameBuffer.data(), blockNameLength);

				m_NamedBlockUniformCount++;
				m_Blocks[name].AddUniform(uniform);
			}
			else
			{
				uint32_t bufferOffset = currentOffset;
				currentOffset += ShaderDataTypeSize(shaderDataType);

				uniform.SetBufferOffsetForDefaultBlockUniform(bufferOffset);

				// Add the uniform to the base block storage.
				m_DefaultBlockUniformCount++;
				m_BaseBlockUniforms[name] = uniform;
			}
		}
	}

	GLint OpenGLShader::UploadUniformBool(const std::string& name, bool value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1f(location, value);
		return location;
	}

	GLint OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1f(location, value);
		return location;
	}

	GLint OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform2f(location, value.x, value.y);
		return location;
	}

	GLint OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
		return location;
	}

	GLint OpenGLShader::UploadUniformFloat3Array(const std::string& name, uint32_t count,
		glm::vec3* value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform3fv(location, count, glm::value_ptr(value[0]));
		return location;
	}

	GLint OpenGLShader::UploadUniformFloat2Array(const std::string& name, uint32_t count,
		glm::vec2* value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform2fv(location, count, glm::value_ptr(value[0]));
		return location;
	}

	GLint OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
		return location;
	}

	GLint OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1i(location, value);
		return location;
	}

	GLint OpenGLShader::UploadUniformIntArray(const std::string& name, uint32_t count, int* basePtr)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniform1iv(location, count, basePtr);
		return location;
	}

	GLint OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		return location;
	}

	GLint OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_ID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		return location;
	}

	void OpenGLShader::EnableShaderImageAccessBarrierBit()
	{
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void OpenGLShader::DispatchCompute(uint32_t groupX, uint32_t groupY, uint32_t groupZ)
	{
		if (!m_IsCompute)
		{
			YGG_LOG_WARN("Shader: '{}' is not of type Compute Shader.  Cannot dispatch.", m_Name);
			return;
		}

		glUseProgram(m_ID);
		glDispatchCompute(groupX, groupY, groupZ);
	}
} // namespace askygg
