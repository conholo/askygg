#pragma once
#include <utility>
#include <vector>
#include <glm/glm.hpp>

#include "askygg/core/Assert.h"

namespace askygg
{
	namespace ImageUtils
	{
		enum class TextureShaderDataFormat;
		enum class TextureAccessLevel;
	} // namespace ImageUtils

	enum class FramebufferTextureFormat
	{
		None = 0,
		RGBA8,
		RGBA32F,
		RED_INTEGER,
		DEPTH24STENCIL8,
		DEPTH32F,

		Depth = DEPTH24STENCIL8,
		Depth32F = DEPTH32F,
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format) {}

		FramebufferTextureFormat TextureFormat;
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(
			std::initializer_list<FramebufferTextureSpecification> fboTextureSpecifications)
			: FBOTextureSpecifications(fboTextureSpecifications) {}

		std::vector<FramebufferTextureSpecification> FBOTextureSpecifications;
	};

	struct FramebufferSpecification
	{
		uint32_t						   Width = 0, Height = 0;
		FramebufferAttachmentSpecification AttachmentSpecification;
		bool							   IsLayered = false;
		uint32_t						   Layers = 0;
	};

	class Framebuffer
	{
	public:
		explicit Framebuffer(FramebufferSpecification spec)
			: m_Specification(std::move(spec)) {}
		virtual ~Framebuffer() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Invalidate() = 0;
		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void ReadColorData(void* pixels, uint32_t attachmentIndex) const = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

	public:
		[[nodiscard]] uint32_t GetColorAttachmentID(uint32_t index = 0) const
		{
			YGG_ASSERT(index <= m_ColorAttachmentIDs.size(), "FBO Error: No color attachment at index: {}",
				index);
			return m_ColorAttachmentIDs[index];
		}
		[[nodiscard]] uint32_t						  GetDepthAttachmentID() const { return m_DepthAttachmentID; }
		[[nodiscard]] uint32_t						  GetLayeredDepthAttachmentID() const { return m_DepthAttachmentID; };
		[[nodiscard]] uint32_t						  GetID() const { return m_ID; }
		[[nodiscard]] const FramebufferSpecification& GetFramebufferSpecification() const
		{
			return m_Specification;
		}
		[[nodiscard]] glm::vec2 GetCurrentSize() const
		{
			return { m_Specification.Width, m_Specification.Height };
		}

	protected:
		FramebufferSpecification m_Specification;
		uint32_t				 m_ID{ UINT32_MAX };

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentTextureSpecs;
		std::vector<uint32_t>						 m_ColorAttachmentIDs;

		FramebufferTextureSpecification m_DepthAttachmentTextureSpec{ FramebufferTextureFormat::None };
		uint32_t						m_DepthAttachmentID = -1;
	};
} // namespace askygg