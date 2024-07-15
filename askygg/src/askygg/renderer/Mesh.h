#pragma once

#include "askygg/renderer/BufferLayout.h"
#include "askygg/renderer/IndexBuffer.h"
#include "askygg/renderer/VertexBuffer.h"
#include "askygg/core/Memory.h"

#include <glm/glm.hpp>
#include <vector>

namespace askygg
{
	struct Vertex
	{
		glm::vec3 Position{ 0.0f };
		glm::vec3 Normal{ 0.0f };
		glm::vec3 Tangent{ 0.0f };
		glm::vec3 Binormal{ 0.0f };
		glm::vec2 TexCoord{ 0.0f };
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(const Mesh&) = default;
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

		const Ref<IndexBuffer>&		 GetIndexBuffer() const { return m_IndexBuffer; }
		const Ref<VertexBuffer>&	 GetVertexBuffer() const { return m_VertexBuffer; }
		const std::vector<Vertex>&	 GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

		void			 Unbind() const;
		void			 Bind() const;
		static Ref<Mesh> CreateQuad();

	private:
		void CreateRenderPrimitives(std::vector<Vertex> vertices, std::vector<uint32_t> indices);

	private:
		std::vector<Vertex>	  m_Vertices;
		std::vector<uint32_t> m_Indices;
		Ref<VertexBuffer>	  m_VertexBuffer;
		Ref<IndexBuffer>	  m_IndexBuffer;
	};
} // namespace askygg