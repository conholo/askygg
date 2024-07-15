#include "askygg/renderer/Mesh.h"

namespace askygg
{
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: m_Vertices(vertices), m_Indices(indices)
	{
		CreateRenderPrimitives(vertices, indices);
	}

	void Mesh::Bind() const
	{
		m_VertexBuffer->Bind();
		m_IndexBuffer->Bind();
	}

	void Mesh::Unbind() const
	{
		m_VertexBuffer->Unbind();
		m_IndexBuffer->Unbind();
	}

	void Mesh::CreateRenderPrimitives(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
	{
		float* basePtr = &vertices.data()->Position.x;
		m_VertexBuffer = VertexBuffer::Create(basePtr, sizeof(Vertex) * vertices.size());

		BufferLayout layout({ { "a_Position", ShaderAttributeType::Float3 },
			{ "a_Normal", ShaderAttributeType::Float3 },
			{ "a_Tangent", ShaderAttributeType::Float3 },
			{ "a_Binormal", ShaderAttributeType::Float3 },
			{ "a_TexCoord", ShaderAttributeType::Float2 } });
		m_VertexBuffer->SetLayout(layout);

		uint32_t* indexPtr = indices.data();
		m_IndexBuffer = IndexBuffer::Create(indexPtr, indices.size());
	}

	Ref<Mesh> Mesh::CreateQuad()
	{
		std::vector<Vertex> vertices = {
			Vertex{ { -1.0f, -1.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f } },
			Vertex{ { 1.0f, -1.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 1.0f, 0.0f } },
			Vertex{ { 1.0f, 1.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 1.0f, 1.0f } },
			Vertex{ { -1.0f, 1.0f, 0.0f },
				{ 0.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 0.0f, 0.0f },
				{ 0.0f, 1.0f } },
		};

		std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

		return CreateRef<Mesh>(vertices, indices);
	}
} // namespace askygg