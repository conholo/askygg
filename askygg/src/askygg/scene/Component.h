#pragma once
#include "askygg/core/UUID.h"
#include "askygg/renderer/Mesh.h"
#include "askygg/renderer/Material.h"

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <utility>

namespace askygg
{
	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(std::string tag)
			: Tag(std::move(tag)) {}
	};

	struct TransformComponent
	{
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& Position, const glm::vec3& Degrees, const glm::vec3& Size)
			: Translation(Position), RotationDegrees(Degrees), Scale(Size) {}

		glm::mat4 Transform() const
		{
			return translate(glm::mat4(1.0), Translation) * toMat4(glm::quat(radians(RotationDegrees))) * scale(glm::mat4(1.0), Scale);
		}

		glm::quat Orientation() const { return glm::quat(radians(RotationDegrees)); }

		glm::vec3 Up() const { return rotate(Orientation(), glm::vec3(0.0f, 1.0f, 0.0f)); }

		glm::vec3 Right() const { return rotate(Orientation(), glm::vec3(1.0f, 0.0f, 0.0f)); }

		glm::vec3 Forward() const { return rotate(Orientation(), glm::vec3(0.0f, 0.0f, -1.0f)); }

		glm::vec3 Translation = glm::vec3(0.0f);
		glm::vec3 RotationDegrees = glm::vec3(0.0f);
		glm::vec3 Scale = glm::vec3(1.0f);
	};

	struct CameraComponent
	{
		glm::mat4 View;
		glm::mat4 Projection;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(const glm::mat4& view, const glm::mat4& projection)
			: View(view), Projection(projection) {}
	};

	struct MaterialComponent
	{
		Ref<Material> MaterialInstance;
		MaterialComponent() = default;
		MaterialComponent(const MaterialComponent&) = default;
		MaterialComponent(const Ref<Material>& material)
			: MaterialInstance(material) {}
	};
} // namespace askygg
