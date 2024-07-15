#pragma once

#include <cstdint>
#include "askygg/renderer/IndexBuffer.h"

namespace askygg
{
	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		explicit OpenGLIndexBuffer(uint32_t count);
		OpenGLIndexBuffer(uint32_t* indices, uint32_t count);
		~OpenGLIndexBuffer() override;

		void Bind() const override;
		void Unbind() const override;

		uint32_t GetID() const override { return m_ID; }
		uint32_t GetIndexCount() const override { return m_Count; }

	private:
		uint32_t m_Count;
		uint32_t m_ID{};
	};
} // namespace askygg
