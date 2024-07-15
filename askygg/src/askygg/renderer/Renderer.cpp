#include "askygg/renderer/Renderer.h"
#include "askygg/renderer/RenderCommand.h"
#include "askygg/renderer/Framebuffer.h"
#include "askygg/renderer/VertexArray.h"
#include "askygg/renderer/Texture.h"
#include "askygg/renderer/UniformBuffer.h"
#include "askygg/renderer/Mesh.h"
#include "askygg/core/Application.h"
#include "askygg/core/Time.h"
#include "askygg/scene/Component.h"
#include "askygg/scene/Entity.h"

namespace askygg
{
	Renderer::Statistics Renderer::RendererStatistics;

	struct RendererData
	{
		struct UBGlobalData
		{
			glm::vec4 Time;
			glm::vec4 Resolution;
		};

		struct UBCameraData
		{
			glm::vec4 CameraPosition;
			glm::mat4 ViewProjectionMatrix;
			glm::mat4 ProjectionMatrix;
			glm::mat4 ViewMatrix;
		};

		struct UBEntityData
		{
			glm::mat4 ModelMatrix;
		};

		Ref<UniformBuffer> GlobalUB;
		Ref<UniformBuffer> CameraUB;
		Ref<UniformBuffer> EntityUB;

		// Must match GLSL & visa versa.
		std::unordered_map<std::string, uint32_t> s_UniformBufferBindingMap = {
			{ TypeName<UBGlobalData>(), 0 },
			{ TypeName<UBCameraData>(), 1 },
			{ TypeName<UBEntityData>(), 2 },
		};

		Ref<Mesh>		 QuadMesh;
		Ref<VertexArray> RendererVAO;
	};
	static RendererData* s_RenderData = nullptr;

	void Renderer::UploadGlobalUBData(const glm::vec2& viewportSize)
	{
		YGG_ASSERT(s_RenderData, "Render Data is invalid.");
		const RendererData::UBGlobalData globalData{
			{ Time::DeltaTime(), Time::Elapsed() / 20.0f, Time::Elapsed(), Time::Elapsed() * 2.0f },
			{ viewportSize.x, viewportSize.y, 0.0f, 0.0f },
		};

		s_RenderData->GlobalUB->SetData(&globalData, sizeof(RendererData::UBGlobalData));
	}

	void Renderer::UploadCameraUBData()
	{
		YGG_ASSERT(s_RenderData, "Render Data is invalid.");

		const glm::mat4 ProjectionMatrix = Camera::GetProjectionMatrix();
		const glm::mat4 ViewMatrix = Camera::GetViewMatrix();
		const glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		const RendererData::UBCameraData CameraData{ glm::vec4(Camera::GetPosition(), 1.0f),
			ViewProjectionMatrix, ProjectionMatrix, ViewMatrix };
		s_RenderData->CameraUB->SetData(&CameraData, sizeof(RendererData::UBCameraData));
	}

	void Renderer::Initialize()
	{
		s_RenderData = new RendererData;
		s_RenderData->RendererVAO = VertexArray::Create();
		s_RenderData->QuadMesh = Mesh::CreateQuad();
		s_RenderData->RendererVAO->SetIndexBuffer(s_RenderData->QuadMesh->GetIndexBuffer());
		s_RenderData->RendererVAO->EnableVertexAttributes(s_RenderData->QuadMesh->GetVertexBuffer());

		uint32_t GlobalUBSlot =
			s_RenderData->s_UniformBufferBindingMap[TypeName<RendererData::UBGlobalData>()];
		s_RenderData->GlobalUB =
			CreateRef<UniformBuffer>(sizeof(RendererData::UBGlobalData), GlobalUBSlot);

		uint32_t CameraUBSlot =
			s_RenderData->s_UniformBufferBindingMap[TypeName<RendererData::UBCameraData>()];
		s_RenderData->CameraUB =
			CreateRef<UniformBuffer>(sizeof(RendererData::UBCameraData), CameraUBSlot);

		uint32_t EntityUBSlot =
			s_RenderData->s_UniformBufferBindingMap[TypeName<RendererData::UBEntityData>()];
		s_RenderData->EntityUB =
			CreateRef<UniformBuffer>(sizeof(RendererData::UBEntityData), EntityUBSlot);

		TextureLibrary::LoadWhiteTexture();
		TextureLibrary::LoadBlackTexture();
		TextureLibrary::LoadBlackTextureCube();
		TextureLibrary::LoadWhiteTextureArray();
	}

	void Renderer::Shutdown()
	{
		ShaderLibrary::Shutdown();
		delete s_RenderData;
	}

	void Renderer::BeginScene(const glm::vec2& viewportSize)
	{
		RendererStatistics.Clear();
		UploadGlobalUBData(viewportSize);
		UploadCameraUBData();
	}

	void Renderer::EndScene() {}

	void Renderer::BeginPass(const Ref<RenderPass>& pass)
	{
		if (pass->GetSpecification().Type != PassType::DefaultFBO)
		{
			const Ref<Framebuffer> PassFB = pass->GetSpecification().PassFramebuffer;
			PassFB->Bind();
		}
		else
		{
			RenderCommand::SetViewport(Application::GetWindow().GetWidth(),
				Application::GetWindow().GetHeight());
		}

		const auto& Specification = pass->GetSpecification();
		RenderCommand::ClearColor(Specification.ClearColor);
		RenderCommand::Clear(Specification.ShouldClearColor, Specification.ShouldClearDepth);
	}

	void Renderer::EndPass(const Ref<RenderPass>& renderPass)
	{
		if (renderPass->GetSpecification().Type == PassType::DefaultFBO)
			return;
		renderPass->GetSpecification().PassFramebuffer->Unbind();
	}

	void Renderer::DrawFullScreenQuad(const Ref<Material>& material)
	{
		s_RenderData->QuadMesh->Bind();
		s_RenderData->RendererVAO->Bind();
		material->GetShader()->Bind();
		{
			material->UploadStagedUniforms();
			RenderCommand::DrawIndexed(s_RenderData->RendererVAO,
				s_RenderData->QuadMesh->GetIndexBuffer()->GetIndexCount());
		}
		material->GetShader()->Unbind();
		s_RenderData->RendererVAO->Unbind();
		s_RenderData->QuadMesh->Unbind();

		RendererStatistics.TriangleCount += s_RenderData->QuadMesh->GetIndices().size() / 3;
		RendererStatistics.VertexCount += s_RenderData->QuadMesh->GetVertices().size();
	}
} // namespace askygg
