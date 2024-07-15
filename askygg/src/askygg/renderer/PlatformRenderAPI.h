#pragma once

#include "VertexArray.h"
#include "Material.h"
#include "RenderPass.h"

#include <glm/glm.hpp>

namespace askygg
{
	class PlatformRenderAPI
	{
	public:
		enum class API
		{
			None = 0,
			OpenGL = 1,
			Metal = 2,
			Vulkan = 3
		};

	public:
		static void				  InitializePlatformRendererType();
		static API				  GetPlatformRendererType() { return s_API; }
		static PlatformRenderAPI* Get() { return s_RenderAPI.get(); };

	public:
		virtual ~PlatformRenderAPI() = default;
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear(bool colorBufferBit, bool depthBufferBit) = 0;
		virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) = 0;

	private:
		static Ref<PlatformRenderAPI> s_RenderAPI;
		static API					  s_API;
	};
} // namespace askygg