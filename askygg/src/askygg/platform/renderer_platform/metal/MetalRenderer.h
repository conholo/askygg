#pragma once

#include "askygg/renderer/PlatformRenderAPI.h"
#include "askygg/core/Memory.h"

#include "MetalKit/MetalKit.hpp"
#include "MetalDevice.h"

namespace askygg
{
	class MetalRenderer : public PlatformRenderAPI
	{
	public:
		MetalRenderer();
		~MetalRenderer() override;

		void Initialize() override;

		void BeginScene(const glm::vec2& outputSize) override;
		void EndScene() override;
		void BeginPass(const Ref<RenderPass>& pass) override;
		void EndPass(const Ref<RenderPass>& renderPass) override;

		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
		void SetClearColor(const glm::vec4& color) override;
		void Clear(bool colorBit, bool depthBit) override;
		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;

	public:
		void DrawView(MTK::View* view);

	private:
		void BuildShaders();
		void BuildBuffers();

	private:
		MTL::RenderPipelineState* m_PSO{};
		MTL::Buffer*			  m_VertexPositionsBuffer{};
		MTL::Buffer*			  m_VertexColorsBuffer{};
		int						  m_VertexCount{ 0 };

		MTL::CommandQueue* m_CommandQueue{};
		Ref<MetalDevice>   m_Device;
	};
} // namespace askygg