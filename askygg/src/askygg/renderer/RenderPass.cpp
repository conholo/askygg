#include "askygg/renderer/RenderPass.h"
#include <utility>

namespace askygg
{
	RenderPass::RenderPass(RenderPassSpecification specification)
		: m_Specification(std::move(specification)) {}
} // namespace askygg
