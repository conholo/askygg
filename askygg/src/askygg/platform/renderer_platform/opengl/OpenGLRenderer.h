#pragma once
#include "askygg/renderer/PlatformRenderAPI.h"

namespace askygg
{
	class OpenGLRenderer : public PlatformRenderAPI
	{
	public:
		void Initialize() override;
		void Shutdown() override;
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear(bool colorBufferBit, bool depthBufferBit) override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
	};
} // namespace askygg