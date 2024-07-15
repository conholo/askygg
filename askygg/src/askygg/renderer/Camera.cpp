
#include "askygg/renderer/Camera.h"
#include "askygg/event/WindowEvent.h"
#include "askygg/core/Input.h"
#include "askygg/core/Utility.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#define PI 3.14159265359

namespace askygg
{
	void Camera::Initialize(const OrthographicProperties& properties)
	{
		s_FOV = properties.FOV;
		s_AspectRatio = static_cast<float>(properties.Width) / static_cast<float>(properties.Height);
		s_NearClip = properties.NearClip;
		s_FarClip = properties.FarClip;
		s_Projection = glm::perspective(glm::radians(s_FOV), s_AspectRatio, s_NearClip, s_FarClip);
		UpdateView();
	}

	void Camera::SetViewportSize(float width, float height)
	{
		// YGG_LOG_INFO("Updating camera viewport: {}, {}", width, height);
		s_ViewportWidth = width;
		s_ViewportHeight = height;
		UpdateProjection();
	}

	void Camera::UpdateProjection()
	{
		s_AspectRatio = s_ViewportWidth / s_ViewportHeight;
		s_Projection = glm::perspective(glm::radians(s_FOV), s_AspectRatio, s_NearClip, s_FarClip);
	}

	void Camera::UpdateView()
	{
		s_Yaw = s_Pitch = 0.0f;
		s_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();
		s_ViewMatrix = glm::translate(glm::mat4(1.0f), s_Position) * glm::toMat4(orientation);
		s_ViewMatrix = glm::inverse(s_ViewMatrix);
	}

	std::pair<float, float> Camera::PanSpeed()
	{
		float x = std::min(s_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(s_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float Camera::ZoomSpeed()
	{
		float distance = s_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void Camera::OnUpdate(float deltaTime)
	{
		if (Input::IsKeyPressed(Key::LeftAlt))
		{
			const glm::vec2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			glm::vec2		 delta = (mouse - s_InitialMousePosition) * 0.003f;
			s_InitialMousePosition = mouse;

			if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
				MouseRotate(delta);
			else if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
				MouseZoom(delta.y);
		}

		UpdateView();
	}

	void Camera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(Camera::OnMouseScroll);
	}

	bool Camera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void Camera::MousePan(const glm::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		s_FocalPoint += -GetRightDirection() * delta.x * xSpeed * s_Distance;
		s_FocalPoint += GetUpDirection() * delta.y * ySpeed * s_Distance;
	}

	void Camera::MouseRotate(const glm::vec2& delta)
	{
		constexpr float RotationSpeed = 0.8f;
		float			yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		s_Yaw += yawSign * delta.x * RotationSpeed;
		s_Pitch += delta.y * RotationSpeed;
	}

	void Camera::MouseZoom(float delta)
	{
		s_Distance -= delta * ZoomSpeed();
		if (s_Distance < 1.0f)
		{
			s_FocalPoint += GetForwardDirection();
			s_Distance = 1.0f;
		}
	}

	glm::vec3 Camera::GetUpDirection()
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Camera::GetRightDirection()
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 Camera::GetForwardDirection()
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 Camera::CalculatePosition()
	{
		return s_FocalPoint - GetForwardDirection() * s_Distance;
	}

	glm::quat Camera::GetOrientation()
	{
		return { glm::vec3(-s_Pitch, -s_Yaw, 0.0f) };
	}
} // namespace askygg