#include "askygg/renderer/RenderCommand.h"
#include "PlatformRenderAPI.h"

namespace askygg
{
	void RenderCommand::Initialize()
	{
		PlatformRenderAPI::Get()->Initialize();
	}

	void RenderCommand::Clear(bool colorBufferBit, bool depthBufferBit)
	{
		PlatformRenderAPI::Get()->Clear(colorBufferBit, depthBufferBit);
	}

	void RenderCommand::SetViewport(uint32_t width, uint32_t height)
	{
		PlatformRenderAPI::Get()->SetViewport(0, 0, width, height);
	}

	void RenderCommand::ClearColor(const glm::vec4& color)
	{
		PlatformRenderAPI::Get()->SetClearColor(color);
	}

	void RenderCommand::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		PlatformRenderAPI::Get()->DrawIndexed(vertexArray, indexCount);
	}
} // namespace askygg
