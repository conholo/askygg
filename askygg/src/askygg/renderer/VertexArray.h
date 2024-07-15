#pragma once

#include "askygg/renderer/VertexBuffer.h"
#include "askygg/renderer/IndexBuffer.h"
#include "askygg/core/Memory.h"

namespace askygg
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() = default;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void EnableVertexAttributes(const Ref<VertexBuffer>& vertexBuffer) const = 0;

		void								  SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer) { m_IndexBuffer = indexBuffer; }
		[[nodiscard]] const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

		static Ref<VertexArray> Create();

	protected:
		Ref<IndexBuffer> m_IndexBuffer;
	};
} // namespace askygg
