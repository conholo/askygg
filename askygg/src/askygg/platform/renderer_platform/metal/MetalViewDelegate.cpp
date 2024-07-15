#include "askygg/core/Assert.h"
#include "askygg/renderer/PlatformRenderAPI.h"
#include "MetalViewDelegate.h"
#include "MetalRenderer.h"

namespace askygg
{
	void MetalViewDelegate::drawInMTKView(MTK::View* view)
	{
		auto* renderer = dynamic_cast<MetalRenderer*>(PlatformRenderAPI::Get());
		YGG_ASSERT(renderer, "Metal Renderer is invalid.");
		renderer->DrawView(view);
	}

	MetalViewDelegate::~MetalViewDelegate() {}
} // namespace askygg