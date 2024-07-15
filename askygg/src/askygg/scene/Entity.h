#pragma once

#include "askygg/core/Utility.h"
#include "askygg/scene/Scene.h"
#include <entt/entt.hpp>

namespace askygg
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template <typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			YGG_ASSERT(!HasComponent<T>(), "Can't add component.  Entity already has component: {}.",
				TypeName<T>());
			T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			m_Scene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template <typename T>
		T& GetComponent()
		{
			YGG_ASSERT(HasComponent<T>(), "Can't get component.  Entity does not have component: {}.",
				TypeName<T>());
			return m_Scene->m_Registry.get<T>(m_EntityHandle);
			;
		}

		template <typename T>
		bool HasComponent() const
		{
			return m_Scene->m_Registry.has<T>(m_EntityHandle);
		}

		template <typename T>
		void RemoveComponent() const
		{
			YGG_ASSERT(HasComponent<T>(), "Can't remove component.  Entity does not have component: {}.",
				TypeName<T>());
			m_Scene->m_Registry.remove<T>(m_EntityHandle);
		}

		UUID			   GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		operator bool() const { return m_EntityHandle != entt::null; }
		operator uint32_t() const { return static_cast<uint32_t>(m_EntityHandle); }
		operator entt::entity() const { return m_EntityHandle; }
		operator UUID() { return GetUUID(); }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
		}

		bool operator!=(const Entity& other) const { return !(*this == other); }

	private:
		Scene*		 m_Scene = nullptr;
		entt::entity m_EntityHandle{ entt::null };
	};
} // namespace askygg
