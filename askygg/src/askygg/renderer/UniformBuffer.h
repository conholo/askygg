#pragma once
#include <cstdint>

namespace askygg
{
	class UniformBuffer
	{
	public:
		UniformBuffer(uint32_t size, uint32_t binding);
		~UniformBuffer();

		void SetData(const void* data, uint32_t size, uint32_t offset = 0);

	private:
		uint32_t m_Binding;
		uint32_t m_ID;
	};
} // namespace askygg