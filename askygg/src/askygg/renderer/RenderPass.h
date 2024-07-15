#pragma once

#include "askygg/core/Memory.h"
#include "askygg/renderer/Framebuffer.h"
#include "askygg/renderer/Material.h"

namespace askygg
{
	enum class PassType
	{
		None = 0,
		DefaultFBO,
		CustomFBO
	};

	struct RenderPassSpecification
	{
		Ref<Material>	 PassMaterial;
		Ref<Framebuffer> PassFramebuffer;
		bool			 ShouldClearDepth = true;
		bool			 ShouldClearColor = true;
		glm::vec4		 ClearColor{ 0.0f };
		PassType		 Type = PassType::CustomFBO;
	};

	class RenderPass
	{
	public:
		explicit RenderPass(RenderPassSpecification specification);
		virtual ~RenderPass() = default;

		RenderPassSpecification&					 GetSpecification() { return m_Specification; }
		[[nodiscard]] const RenderPassSpecification& GetSpecification() const { return m_Specification; }

	protected:
		RenderPassSpecification m_Specification;
	};
} // namespace askygg