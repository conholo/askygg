#pragma once

#include "askygg/core/KeyCodes.h"
#include "askygg/core/MouseCodes.h"

#include <glm/glm.hpp>

namespace askygg
{
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);

		static glm::vec2 GetMousePosition();
		static float	 GetMouseX();
		static float	 GetMouseY();
	};
} // namespace askygg