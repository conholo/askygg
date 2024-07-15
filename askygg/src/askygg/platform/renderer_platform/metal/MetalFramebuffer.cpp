#include "askygg/platform/renderer_platform/metal/MetalFramebuffer.h"

namespace askygg
{
	MetalFramebuffer::MetalFramebuffer(const askygg::FramebufferSpecification& spec)
		: Framebuffer(spec) {}

	MetalFramebuffer::~MetalFramebuffer()
	{
		m_RenderTexture->release();
		m_RenderPassDescriptor->release();
	}
} // namespace askygg