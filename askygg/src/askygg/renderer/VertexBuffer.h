#pragma once
#include "askygg/renderer/BufferLayout.h"
#include "askygg/core/Memory.h"

namespace askygg
{
	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void SetData(const void* data, uint32_t size) = 0;
		virtual void Resize(uint32_t size) = 0;

		virtual void				   Bind() const = 0;
		virtual void				   Unbind() const = 0;
		[[nodiscard]] virtual uint32_t GetID() const = 0;
		virtual void				   SetLayout(const BufferLayout& layout) { m_Layout = layout; }
		const BufferLayout&			   GetLayout() const { return m_Layout; }

		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);

	protected:
		BufferLayout m_Layout;
	};
} // namespace askygg
