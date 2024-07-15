#pragma once

#include "askygg/platform/renderer_platform/opengl/OpenGLIndexBuffer.h"
#include "askygg/platform/renderer_platform/opengl/OpenGLVertexBuffer.h"
#include "askygg/core/Memory.h"
#include "askygg/renderer/VertexArray.h"

namespace askygg
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray() override;

		void Bind() const override;
		void Unbind() const override;
		void EnableVertexAttributes(const Ref<VertexBuffer>& vertexBuffer) const override;

	private:
		uint32_t m_ID{};
	};
} // namespace askygg
