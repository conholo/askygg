#include <chrono>
#include <iostream>
#include <utility>
#include "askygg/core/Log.h"

namespace askygg
{
	class ScopedTimer
	{
	public:
		enum class Unit
		{
			Hours,
			Minutes,
			Seconds,
			Milliseconds,
			Microseconds,
			Nanoseconds
		};

		explicit ScopedTimer(std::string name, Unit unit = Unit::Milliseconds)
			: m_Name(std::move(name)), m_Unit(unit), m_Stopped(false)
		{
			m_StartTimepoint = std::chrono::high_resolution_clock::now();
		}

		~ScopedTimer()
		{
			if (!m_Stopped)
				Stop();
		}

		void Stop()
		{
			auto endTimepoint = std::chrono::high_resolution_clock::now();

			long long start = std::chrono::time_point_cast<std::chrono::nanoseconds>(m_StartTimepoint)
								  .time_since_epoch()
								  .count();
			long long end = std::chrono::time_point_cast<std::chrono::nanoseconds>(endTimepoint)
								.time_since_epoch()
								.count();

			switch (m_Unit)
			{
				case Unit::Hours:
					YGG_LOG_INFO("'{}' execution time: {}h", m_Name, (end - start) * 1e-9 / 3600.0);
					break;
				case Unit::Minutes:
					YGG_LOG_INFO("'{}' execution time: {}m", m_Name, (end - start) * 1e-9 / 60.0);
					break;
				case Unit::Seconds:
					YGG_LOG_INFO("'{}' execution time: {}s ", m_Name, (end - start) * 1e-9);
					break;
				case Unit::Milliseconds:
					YGG_LOG_INFO("'{}' execution time: {}ms ", m_Name, (end - start) * 1e-6);
					break;
				case Unit::Microseconds:
					YGG_LOG_INFO("'{}' execution time: {}us ", m_Name, (end - start) * 1e-3);
					break;
				case Unit::Nanoseconds:
					YGG_LOG_INFO("'{}' execution time: {}ns ", m_Name, (end - start));
					break;
			}

            m_NanoSeconds = end - start;
			m_Stopped = true;
		}

        long long int GetNanoSeconds() const { return m_NanoSeconds; }

	private:
		std::string													m_Name;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
		bool														m_Stopped;
		Unit														m_Unit;
        long long int m_NanoSeconds;
    };
}
