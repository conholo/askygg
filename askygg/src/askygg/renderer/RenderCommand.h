#pragma once

#include <glm/glm.hpp>
#include "askygg/renderer/VertexArray.h"

namespace askygg
{
	enum class RenderFlag
	{
		None = (1 << 0),
		DepthTest = (1 << 1),
		Blend = (1 << 2),
		TwoSided = (1 << 3)
	};

	enum class DepthFlag
	{
		Never,
		Less,
		Equal,
		LEqual,
		Greater,
		NotEqual,
		GEqual,
		Always
	};
	enum class DrawMode
	{
		None = 0,
		Fill,
		WireFrame
	};
	enum class FaceCullMode
	{
		None = 0,
		Front,
		Back
	};

	class RenderCommand
	{
	public:
		static void Initialize();
		static void Clear(bool colorBufferBit, bool depthBufferBit);
		static void SetViewport(uint32_t width, uint32_t height);
		static void ClearColor(const glm::vec4& clearColor);
		static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0);
	};
} // namespace askygg
