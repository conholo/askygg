#include "askygg/core/Input.h"
#include "askygg/core/Application.h"

#include <GLFW/glfw3.h>

namespace askygg
{
	bool Input::IsKeyPressed(KeyCode key)
	{
		auto*	  window = static_cast<GLFWwindow*>(Application::GetWindow().GetNativeWindow());
		const int action = glfwGetKey(window, key);

		return action == GLFW_PRESS || action == GLFW_REPEAT;
	}

	bool Input::IsMouseButtonPressed(MouseCode button)
	{
		auto*	  window = static_cast<GLFWwindow*>(Application::GetWindow().GetNativeWindow());
		const int action = glfwGetMouseButton(window, button);

		return action == GLFW_PRESS || action == GLFW_REPEAT;
	}

	glm::vec2 Input::GetMousePosition()
	{
		auto* window = static_cast<GLFWwindow*>(Application::GetWindow().GetNativeWindow());

		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		return { (float)xPos, (float)yPos };
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}
} // namespace askygg