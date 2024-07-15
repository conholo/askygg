#include "askygg/scene/Entity.h"

namespace askygg
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_Scene(scene), m_EntityHandle(handle) {}
} // namespace askygg