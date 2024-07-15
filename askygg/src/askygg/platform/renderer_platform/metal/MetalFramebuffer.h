#pragma once

#include "askygg/renderer/Framebuffer.h"
#include "Metal/MTLTexture.hpp"
#include "Metal/MTLRenderPass.hpp"

namespace askygg
{
	class MetalFramebuffer : public Framebuffer
	{
	public:
		explicit MetalFramebuffer(const FramebufferSpecification& spec);
		~MetalFramebuffer() override;

		void Bind() const override;
		void Unbind() const override;

	private:
		MTL::Texture*			   m_RenderTexture;
		MTL::RenderPassDescriptor* m_RenderPassDescriptor;
	};
} // namespace askygg