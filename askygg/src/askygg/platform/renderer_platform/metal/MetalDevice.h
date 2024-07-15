#pragma once

#include "Metal/Metal.hpp"
#include "AppKit/AppKit.hpp"
#include "MetalKit/MetalKit.hpp"

namespace askygg
{
	class MetalDevice
	{
	public:
		MetalDevice();
		~MetalDevice();

		MTL::Device&	   GetDevice() { return *m_Device; }
		MTL::CommandQueue* CreateCommandQueue();

	private:
		MTL::Device* m_Device;
	};
} // namespace askygg