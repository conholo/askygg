#pragma once

#include "askygg/renderer/Shader.h"
#include "askygg/renderer/Camera.h"
#include "askygg/renderer/RenderPass.h"
#include "askygg/scene/Component.h"
#include "askygg/scene/Scene.h"

namespace askygg
{
	class Renderer
	{
	public:
		static void Initialize();
		static void Shutdown();

		static void BeginScene(const glm::vec2& viewportSize);
		static void BeginPass(const Ref<RenderPass>& pass);
		static void EndPass(const Ref<RenderPass>& renderPass);
		static void EndScene();
		static void DrawFullScreenQuad(const Ref<Material>& material);

		struct Statistics
		{
			uint64_t TriangleCount;
			uint64_t VertexCount;

			void Clear()
			{
				TriangleCount = 0;
				VertexCount = 0;
			}
		};

		static Statistics RendererStatistics;

	private:
		static void UploadGlobalUBData(const glm::vec2& viewportSize);
		static void UploadCameraUBData();
	};
} // namespace askygg