#include "MetalDevice.h"

namespace askygg
{
	MetalDevice::MetalDevice()
	{
		m_Device = MTL::CreateSystemDefaultDevice();
	}

	MetalDevice::~MetalDevice()
	{
		m_Device->release();
	}

	MTL::CommandQueue* MetalDevice::CreateCommandQueue()
	{
		return m_Device->newCommandQueue();
	}
} // namespace askygg