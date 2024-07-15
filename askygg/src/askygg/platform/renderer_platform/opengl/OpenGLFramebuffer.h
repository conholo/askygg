#pragma once

#include "askygg/renderer/Framebuffer.h"

namespace askygg
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		explicit OpenGLFramebuffer(FramebufferSpecification spec);
		~OpenGLFramebuffer() override;

		void Bind() const override;
		void Unbind() const override;
		void Invalidate() override;
		void Resize(uint32_t width, uint32_t height) override;
		void ReadColorData(void* pixels, uint32_t attachmentIndex) const override;
	};
} // namespace askygg