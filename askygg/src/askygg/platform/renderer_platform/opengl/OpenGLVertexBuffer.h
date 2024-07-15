#pragma once

#include "askygg/renderer/BufferLayout.h"
#include "askygg/renderer/VertexBuffer.h"

namespace askygg
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		explicit OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(float* vertices, uint32_t size);
		~OpenGLVertexBuffer() override;

		void SetData(const void* data, uint32_t size) override;
		void Resize(uint32_t size) override;

		void Bind() const override;
		void Unbind() const override;

		uint32_t GetID() const override { return m_ID; }

	private:
		uint32_t m_ID = 0;
	};
} // namespace askygg
