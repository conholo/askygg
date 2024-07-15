#pragma once

#include "askygg/core/UUID.h"
#include "askygg/scene/Component.h"
#include "askygg/renderer/Camera.h"
#include <entt/entt.hpp>

namespace askygg
{
	namespace UI
	{
		class SceneHierarchyPanel;
	}

	class Entity;

	class Scene
	{
	public:
		Scene(std::string sceneName);

		Entity CreateEntity(const std::string& Name = "Entity");
		void   Destroy(Entity entity);

		template <typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

	private:
		Entity CreateEntityWithUUID(UUID UUID, const std::string& Name);
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);

		// Contains Nodes
		entt::registry m_Registry;
		std::string	   m_SceneName;

		friend class Entity;
		friend class ImageEditor;
		friend class UI::SceneHierarchyPanel;
	};
} // namespace askygg
