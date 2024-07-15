#pragma once
#include "askygg/core/Memory.h"

namespace askygg
{
	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;
		virtual void				   Bind() const = 0;
		virtual void				   Unbind() const = 0;
		[[nodiscard]] virtual uint32_t GetID() const = 0;
		[[nodiscard]] virtual uint32_t GetIndexCount() const = 0;

		static Ref<IndexBuffer> Create(uint32_t count);
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};
} // namespace askygg
