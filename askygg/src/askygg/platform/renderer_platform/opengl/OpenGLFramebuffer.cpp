#include "askygg/core/Assert.h"
#include "askygg/platform/renderer_platform/opengl/OpenGLFramebuffer.h"
#include "askygg/renderer/TextureUtils.h"
#include <glad/glad.h>
#include <utility>

namespace askygg
{
	static GLenum TextureFormatToGLenum(FramebufferTextureFormat format)
	{
		switch (format)
		{
			case FramebufferTextureFormat::RGBA32F:
			case FramebufferTextureFormat::RGBA8:
			{
				return GL_RGBA;
			}
			case FramebufferTextureFormat::RED_INTEGER:
			{
				return GL_RED_INTEGER;
			}
			case FramebufferTextureFormat::DEPTH24STENCIL8:
			{
				return GL_DEPTH24_STENCIL8;
			}
			default:
				return 0;
		}
	}

	static void AttachColorTexture(uint32_t attachmentId, GLenum internalFormat, GLenum dataFormat,
		GLenum dataType, uint32_t width, uint32_t height, uint32_t index)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, dataType, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, GL_TEXTURE_2D, attachmentId,
			0);
	}

	static void AttachDepthTexture(uint32_t attachmentId, GLenum internalFormat,
		GLenum depthAttachmentType, uint32_t width, uint32_t height)
	{
		glTexStorage2D(GL_TEXTURE_2D, 1, internalFormat, width, height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glFramebufferTexture2D(GL_FRAMEBUFFER, depthAttachmentType, GL_TEXTURE_2D, attachmentId, 0);
	}

	static void AttachLayeredDepthTarget(uint32_t AttachmentID, GLenum InternalFormat, uint32_t Width,
		uint32_t Height, uint32_t Layers)
	{
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, InternalFormat, Width, Height, Layers, 0, GL_DEPTH_COMPONENT,
			GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		constexpr float BorderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, BorderColor);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, AttachmentID, 0);
	}

	static bool IsDepthFormat(const FramebufferTextureFormat Format)
	{
		return Format == FramebufferTextureFormat::DEPTH24STENCIL8 || Format == FramebufferTextureFormat::DEPTH32F;
	}

	OpenGLFramebuffer::OpenGLFramebuffer(FramebufferSpecification spec)
		: Framebuffer(std::move(spec))
	{
		for (auto textureFormatSpecification :
			m_Specification.AttachmentSpecification.FBOTextureSpecifications)
		{
			if (!IsDepthFormat(textureFormatSpecification.TextureFormat))
				m_ColorAttachmentTextureSpecs.emplace_back(textureFormatSpecification);
			else
				m_DepthAttachmentTextureSpec = textureFormatSpecification;
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_ID);
	}

	void OpenGLFramebuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void OpenGLFramebuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Invalidate()
	{
		if (m_ID)
		{
			glDeleteFramebuffers(1, &m_ID);
			glDeleteTextures(m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());
			glDeleteTextures(1, &m_DepthAttachmentID);

			m_ColorAttachmentIDs.clear();
			m_DepthAttachmentID = 0;
		}

		glCreateFramebuffers(1, &m_ID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_ID);

		if (!m_ColorAttachmentTextureSpecs.empty())
		{
			m_ColorAttachmentIDs.resize(m_ColorAttachmentTextureSpecs.size());

			glCreateTextures(GL_TEXTURE_2D, m_ColorAttachmentIDs.size(), m_ColorAttachmentIDs.data());

			for (size_t i = 0; i < m_ColorAttachmentIDs.size(); i++)
			{
				glBindTexture(GL_TEXTURE_2D, m_ColorAttachmentIDs[i]);

				switch (m_ColorAttachmentTextureSpecs[i].TextureFormat)
				{
					case FramebufferTextureFormat::RGBA8:
					{
						AttachColorTexture(m_ColorAttachmentIDs[i], GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,
							m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FramebufferTextureFormat::RGBA32F:
					{
						AttachColorTexture(m_ColorAttachmentIDs[i], GL_RGBA32F, GL_RGBA, GL_FLOAT,
							m_Specification.Width, m_Specification.Height, i);
						break;
					}
					case FramebufferTextureFormat::RED_INTEGER:
					{
						AttachColorTexture(m_ColorAttachmentIDs[i], GL_R32I, GL_RED_INTEGER, GL_INT,
							m_Specification.Width, m_Specification.Height, i);
						break;
					}
					default:
						break;
				}
			}
		}

		if (m_DepthAttachmentTextureSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			if (m_Specification.IsLayered)
			{
				glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &m_DepthAttachmentID);
				glBindTexture(GL_TEXTURE_2D_ARRAY, m_DepthAttachmentID);
				AttachLayeredDepthTarget(m_DepthAttachmentID, GL_DEPTH_COMPONENT32F, m_Specification.Width,
					m_Specification.Height, m_Specification.Layers);
			}
			else
			{
				glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachmentID);
				glBindTexture(GL_TEXTURE_2D, m_DepthAttachmentID);

				if (m_DepthAttachmentTextureSpec.TextureFormat == FramebufferTextureFormat::DEPTH24STENCIL8)
					AttachDepthTexture(m_DepthAttachmentID, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT,
						m_Specification.Width, m_Specification.Height);
				else if (m_DepthAttachmentTextureSpec.TextureFormat == FramebufferTextureFormat::DEPTH32F)
					AttachDepthTexture(m_DepthAttachmentID, GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT,
						m_Specification.Width, m_Specification.Height);
			}
		}

		if (m_ColorAttachmentIDs.size() > 1)
		{
			YGG_ASSERT(m_ColorAttachmentIDs.size() <= 4,
				"4 Color attachments is the max # of attachments per FBO.");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
				GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachmentIDs.size(), buffers);
		}
		else if (m_ColorAttachmentIDs.empty())
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		const bool CompleteFBO = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		YGG_ASSERT(CompleteFBO, "PassFramebuffer Incomplete.");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	void OpenGLFramebuffer::ReadColorData(void* pixels, uint32_t attachmentIndex) const
	{
		YGG_ASSERT(attachmentIndex <= m_ColorAttachmentIDs.size() - 1,
			"Invalid attachment index to read color data from.");
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		glReadPixels(0, 0, m_Specification.Width, m_Specification.Height, GL_RGB, GL_FLOAT, pixels);
	}
} // namespace askygg
