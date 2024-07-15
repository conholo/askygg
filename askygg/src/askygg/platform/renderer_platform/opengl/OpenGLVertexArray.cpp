#include "askygg/platform/renderer_platform/opengl/OpenGLVertexArray.h"
#include "askygg/renderer/BufferLayout.h"
#include <glad/glad.h>

namespace askygg
{
	GLenum GLEnumFromShaderDataType(ShaderAttributeType type)
	{
		switch (type)
		{
			case ShaderAttributeType::Mat3:
			case ShaderAttributeType::Mat4:
			case ShaderAttributeType::Float:
			case ShaderAttributeType::Float2:
			case ShaderAttributeType::Float3:
			case ShaderAttributeType::Float4:
				return GL_FLOAT;
			case ShaderAttributeType::Int:
				return GL_INT;
			default:
				return GL_FLOAT;
		}
	}

	OpenGLVertexArray::OpenGLVertexArray()
	{
		glCreateVertexArrays(1, &m_ID);
		glBindVertexArray(m_ID);
	}

	OpenGLVertexArray::~OpenGLVertexArray()
	{
		glDeleteVertexArrays(1, &m_ID);
	}

	void OpenGLVertexArray::Bind() const
	{
		glBindVertexArray(m_ID);
	}

	void OpenGLVertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	void OpenGLVertexArray::EnableVertexAttributes(const Ref<VertexBuffer>& vertexBuffer) const
	{
		glBindVertexArray(m_ID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();

		uint32_t index = 0;
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(index);
			glVertexAttribPointer(
				index, static_cast<GLsizei>(element.GetComponentCount()),
				GLEnumFromShaderDataType(element.Type), element.Normalized ? GL_TRUE : GL_FALSE,
				static_cast<GLsizei>(layout.GetStride()), reinterpret_cast<const void*>(element.Offset));
			index++;
		}
	}
} // namespace askygg
