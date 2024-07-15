#pragma once

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

namespace askygg
{
	class MetalViewDelegate : public MTK::ViewDelegate
	{
	public:
		MetalViewDelegate() = default;
		~MetalViewDelegate() override;

		void drawInMTKView(MTK::View* view) override;
	};
} // namespace askygg
