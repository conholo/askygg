#include "askygg/platform/renderer_platform/metal/MetalRenderer.h"
#include "askygg/core/Assert.h"
#include "askygg/core/Application.h"
#include "askygg/platform/mac_os/MacOSApplication.h"

#include <iostream>
#include <string>
#include <fstream>
#include <simd/simd.h>

#include <Metal/Metal.hpp>
#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

namespace askygg
{
	MetalRenderer::MetalRenderer() {}

	MetalRenderer::~MetalRenderer()
	{
		m_CommandQueue->release();
	}

	void MetalRenderer::Initialize()
	{
		auto* macOSApp = dynamic_cast<MacOSApplication*>(Application::GetPlatformAppInterface());
		m_CommandQueue = macOSApp->GetMetalAppDelegate().GetDevice().CreateCommandQueue();
		BuildShaders();
		BuildBuffers();
	}

	void MetalRenderer::DrawView(MTK::View* view)
	{
		NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

		MTL::CommandBuffer* commandBuffer = m_CommandQueue->commandBuffer();
		// Creating a MetalFramebuffer instance
		FramebufferSpecification spec;
		spec.width = view->drawableSize().width;
		spec.height = view->drawableSize().height;
		MetalFramebuffer framebuffer(spec);

		// Using framebuffer's render pass descriptor instead of view's
		MTL::RenderPassDescriptor* renderPassDescriptor = framebuffer.getRenderPassDescriptor();
		// You need to add getRenderPassDescriptor() method to MetalFramebuffer class

		MTL::RenderCommandEncoder* commandEncoder =
			commandBuffer->renderCommandEncoder(renderPassDescriptor);

		commandEncoder->setRenderPipelineState(m_PSO);
		commandEncoder->setVertexBuffer(m_VertexPositionsBuffer, 0, 0);
		commandEncoder->setVertexBuffer(m_VertexColorsBuffer, 0, 1);
		commandEncoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0),
			NS::UInteger(m_VertexCount));

		commandEncoder->endEncoding();
		commandBuffer->presentDrawable(view->currentDrawable());
		commandBuffer->commit();

		pool->release();
	}

	void MetalRenderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {}

	void MetalRenderer::SetClearColor(const glm::vec4& color) {}

	void MetalRenderer::Clear(bool colorBit, bool depthBit) {}

	void MetalRenderer::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) {}

	std::string ReadFileIntoString(const std::string& fileName)
	{
		std::string data;

		std::fstream shaderFile(fileName, std::ios_base::in);

		if (shaderFile)
		{
			shaderFile.seekg(0, std::ios::end);
			data.resize(shaderFile.tellg());
			shaderFile.seekg(0, std::ios::beg);
			shaderFile.read(&data[0], (long)data.size());
		}
		shaderFile.close();

		return data;
	}

	void MetalRenderer::BuildShaders()
	{
		using NS::StringEncoding::UTF8StringEncoding;

		using NS::StringEncoding::UTF8StringEncoding;

		// load shader file
		std::string shaderFile = std::move(ReadFileIntoString(
			"/Users/connorwendt/dev/cxx/askygg/askygg_editor/assets/shaders/metal_shaders/shader.metal"));

		NS::Error*	  error = nullptr;
		auto*		  macOSApp = dynamic_cast<MacOSApplication*>(Application::GetPlatformAppInterface());
		auto&		  device = macOSApp->GetMetalAppDelegate().GetDevice().GetDevice();
		MTL::Library* library = device.newLibrary(
			NS::String::string(shaderFile.c_str(), UTF8StringEncoding), nullptr, &error);

		YGG_ASSERT(library, error->localizedDescription()->utf8String());

		MTL::Function* vertexFn =
			library->newFunction(NS::String::string("vertexMain", UTF8StringEncoding));
		MTL::Function* fragFn =
			library->newFunction(NS::String::string("fragmentMain", UTF8StringEncoding));

		MTL::RenderPipelineDescriptor* pipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
		pipelineDesc->setVertexFunction(vertexFn);
		pipelineDesc->setFragmentFunction(fragFn);
		pipelineDesc->colorAttachments()->object(0)->setPixelFormat(
			MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB);

		m_PSO = device.newRenderPipelineState(pipelineDesc, &error);
		YGG_ASSERT(m_PSO, error->localizedDescription()->utf8String());

		vertexFn->release();
		fragFn->release();
		pipelineDesc->release();
		library->release();
	}

	void MetalRenderer::BuildBuffers()
	{
		// number of total vertices
		const int numOfVertices = 6;

		// buffer of vertex position data in a float type
		simd::float3 positions[numOfVertices] = { { -0.5f, 0.5f, 0.0f }, { -0.5f, -0.5f, 0.0f },
			{ 0.5f, 0.5f, 0.0f }, { 0.5f, 0.5f, 0.0f },
			{ 0.5f, -0.5f, 0.0f }, { -0.5f, -0.5f, 0.0f } };

		// buffer of vertex color data in a flat type
		simd::float3 colors[numOfVertices] = { { 0.0f, 0.0f, 1.0f }, { .5f, 0.0, 0.0f },
			{ 0.0f, 0.6f, 0.0f }, { 0.1f, 0.0f, 0.9f },
			{ 0.1f, 0.4, 0.0f }, { 0.3f, 0.6f, 0.5f } };

		// size of the position data buffer
		int positionsDataSize = numOfVertices * sizeof(simd::float3);
		// size of the color data buffer
		int colorDataSize = numOfVertices * sizeof(simd::float3);

		// buffer to store data on the gpu
		auto* macOSApp = dynamic_cast<MacOSApplication*>(Application::GetPlatformAppInterface());
		auto& device = macOSApp->GetMetalAppDelegate().GetDevice().GetDevice();
		m_VertexPositionsBuffer = device.newBuffer(positionsDataSize, MTL::ResourceStorageModeManaged);
		m_VertexColorsBuffer = device.newBuffer(colorDataSize, MTL::ResourceStorageModeManaged);

		// copy data from cpu buffer to gpu buffer
		memcpy(m_VertexPositionsBuffer->contents(), positions, positionsDataSize);
		memcpy(m_VertexColorsBuffer->contents(), colors, colorDataSize);

		// tell metal that we modify the buffer
		m_VertexPositionsBuffer->didModifyRange(NS::Range::Make(0, m_VertexPositionsBuffer->length()));
		m_VertexColorsBuffer->didModifyRange(NS::Range::Make(0, m_VertexColorsBuffer->length()));
		m_VertexCount = numOfVertices;
	}
} // namespace askygg