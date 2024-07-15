#pragma once

#include "askygg/core/Memory.h"

namespace askygg
{
	class GraphicsContext
	{
	public:
		virtual ~GraphicsContext() = default;
		virtual void				Initialize() = 0;
		virtual void				SwapBuffers() = 0;
		static Ref<GraphicsContext> Create(void* windowHandle);
	};
} // namespace askygg