#pragma once

#include <cstddef>	  // for std::size_t
#include <functional> // for std::hash
#include <cstdint>

namespace askygg
{
	class UUID
	{
	public:
		UUID();
		explicit UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		explicit operator uint64_t() const { return m_UUID; }

	private:
		uint64_t m_UUID;
	};
} // namespace askygg

namespace std
{
	template <>
	struct hash<askygg::UUID>
	{
		std::size_t operator()(const askygg::UUID& uuid) const noexcept
		{
			return static_cast<std::size_t>(static_cast<uint64_t>(uuid));
		}
	};
} // namespace std
