#pragma once

#include "askygg/core/Log.h"
#include <glad/glad.h>

namespace askygg
{
	class OpenGLFuncTimer
	{
	public:
		template <typename Func, typename... Args>
		double ProfileFn(Func openglCalls, Args... args)
		{
			GLuint64	 startTime, stopTime;
			unsigned int queryID[2];

			// generate two queries
			glGenQueries(2, queryID);

			glQueryCounter(queryID[0], GL_TIMESTAMP);

			// Execute OpenGL calls
			openglCalls(args...);

			glQueryCounter(queryID[1], GL_TIMESTAMP);

			// wait until the results are available
			int stopTimerAvailable = 0;
			while (!stopTimerAvailable)
			{
				glGetQueryObjectiv(queryID[1], GL_QUERY_RESULT_AVAILABLE, &stopTimerAvailable);
			}

			// get query results
			glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &startTime);
			glGetQueryObjectui64v(queryID[1], GL_QUERY_RESULT, &stopTime);

			return static_cast<double>(stopTime - startTime) / 1000000.0;
		}
	};
} // namespace askygg