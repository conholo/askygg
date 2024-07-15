#include <utility>

#include "askygg/scene/Scene.h"
#include "askygg/scene/Entity.h"

namespace askygg
{
	Scene::Scene(std::string sceneName)
		: m_SceneName(std::move(sceneName)) {}

	Entity Scene::CreateEntity(const std::string& Name)
	{
		return CreateEntityWithUUID(UUID(), Name);
	}

	Entity Scene::CreateEntityWithUUID(UUID UUID, const std::string& Name)
	{
		Entity Entity = { m_Registry.create(), this };
		Entity.AddComponent<IDComponent>(UUID);
		Entity.AddComponent<TransformComponent>();
		auto& tag = Entity.AddComponent<TagComponent>();
		tag.Tag = Name.empty() ? "Entity" : Name;
		return Entity;
	}

	void Scene::Destroy(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template <>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) {}

	template <>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) {}

	template <>
	void Scene::OnComponentAdded<MaterialComponent>(Entity entity, MaterialComponent& component) {}

	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component) {}

	template <>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component) {}
} // namespace askygg
