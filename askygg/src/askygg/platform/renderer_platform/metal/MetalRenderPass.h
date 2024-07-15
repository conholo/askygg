#include "askygg/renderer/RenderPass.h"

namespace askygg
{
	class MetalRenderPass : public RenderPass
	{
	public:
		MetalRenderPass(const RenderPassSpecification& spec);

		~MetalRenderPass() override;
		void Begin() override;
		void End() override;

	private:
		Ref<MetalFramebuffer>		m_Framebuffer;
		id<MTLCommandBuffer>		m_CommandBuffer{};
		id<MTLRenderCommandEncoder> m_CommandEncoder{};
	};
} // namespace askygg
