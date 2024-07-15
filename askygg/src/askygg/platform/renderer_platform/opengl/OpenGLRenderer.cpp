#include "askygg/platform/renderer_platform/opengl/OpenGLRenderer.h"
#include "askygg/core/Assert.h"
#include "glad/glad.h"

namespace askygg
{
	void OpenGLMessageCallback(unsigned source, unsigned type, unsigned id, unsigned severity,
		int length, const char* message, const void* userParam)
	{
		std::string sourceStr;
		std::string typeStr;
		std::string severityStr;

		switch (source)
		{
			case GL_DEBUG_SOURCE_API:
				sourceStr = "API";
				break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
				sourceStr = "WINDOW SYSTEM";
				break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER:
				sourceStr = "SHADER COMPILER";
				break;
			case GL_DEBUG_SOURCE_THIRD_PARTY:
				sourceStr = "THIRD PARTY";
				break;
			case GL_DEBUG_SOURCE_APPLICATION:
				sourceStr = "APPLICATION";
				break;
			case GL_DEBUG_SOURCE_OTHER:
				sourceStr = "OTHER";
				break;
		}

		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR:
				typeStr = "ERROR";
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				typeStr = "DEPRECATED BEHAVIOR";
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				typeStr = "UNDEFINED BEHAVIOR";
				break;
			case GL_DEBUG_TYPE_PORTABILITY:
				typeStr = "PORTABILITY";
				break;
			case GL_DEBUG_TYPE_PERFORMANCE:
				typeStr = "PERFORMANCE";
				break;
			case GL_DEBUG_TYPE_OTHER:
				typeStr = "OTHER";
				break;
		}

		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:
				severityStr = "HIGH";
				break;
			case GL_DEBUG_SEVERITY_MEDIUM:
				severityStr = "MEDIUM";
				break;
			case GL_DEBUG_SEVERITY_LOW:
				severityStr = "LOW";
				break;
			case GL_DEBUG_SEVERITY_NOTIFICATION:
				severityStr = "NOTIFICATION";
				break;
		}

		// todo (connor): Bogus error on shutdown - hacky and needs a formal fix
		// https://stackoverflow.com/questions/15722803/opengl-shader-error-1282
		if (id == 1282)
			return;

		std::string logMsg = "Source: " + sourceStr + " Type: " + typeStr + " Severity: " + severityStr + " ID: " + std::to_string(id) + " Message: " + message;

		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH:
				YGG_LOG_CRITICAL(logMsg.c_str());
				return;
			case GL_DEBUG_SEVERITY_MEDIUM:
				YGG_LOG_ERROR(logMsg.c_str());
				return;
			case GL_DEBUG_SEVERITY_LOW:
				YGG_LOG_WARN(logMsg.c_str());
				return;
			case GL_DEBUG_SEVERITY_NOTIFICATION:
				YGG_LOG_TRACE(logMsg.c_str());
				return;
		}

		YGG_ASSERT(false, "Unknown severity level!");
	}
	void OpenGLRenderer::Initialize()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OpenGLMessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL,
			GL_FALSE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
	}

	void OpenGLRenderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRenderer::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRenderer::Clear(bool colorBufferBit, bool depthBufferBit)
	{
		GLbitfield flags = 0;

		if (colorBufferBit)
			flags |= GL_COLOR_BUFFER_BIT;

		if (depthBufferBit)
			flags |= GL_DEPTH_BUFFER_BIT;

		if (flags != 0)
			glClear(flags);
	}

	void OpenGLRenderer::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetIndexCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRenderer::Shutdown() {}
} // namespace askygg