#pragma once

#include "askygg/event/Event.h"
#include "askygg/event/MouseEvent.h"
#include "askygg/event/WindowEvent.h"

#include <glm/glm.hpp>

namespace askygg
{
	struct OrthographicProperties
	{
		float	 FOV = 45.0f;
		uint32_t Width = 1280;
		uint32_t Height = 720;
		float	 NearClip = 0.1f;
		float	 FarClip = 1000.0f;
	};

	class Camera
	{
	public:
		Camera() = delete;
		static void Initialize(const OrthographicProperties& properties = OrthographicProperties());

		static void OnUpdate(float deltaTime);
		static void OnEvent(Event& e);

		static float GetDistance() { return s_Distance; }
		static void	 SetDistance(float distance) { s_Distance = distance; }

		static glm::vec2 GetViewportSize() { return { s_ViewportWidth, s_ViewportHeight }; }
		static void		 SetViewportSize(float width, float height);

		static glm::mat4& GetViewMatrix() { return s_ViewMatrix; }
		static glm::mat4& GetProjectionMatrix() { return s_Projection; }
		static glm::mat4  GetViewProjection() { return s_Projection * s_ViewMatrix; }

		static glm::vec3  GetUpDirection();
		static glm::vec3  GetRightDirection();
		static glm::vec3  GetForwardDirection();
		static glm::vec3& GetPosition() { return s_Position; }
		static glm::quat  GetOrientation();

		static float GetPitch() { return s_Pitch; }
		static float GetYaw() { return s_Yaw; }

	private:
		static void UpdateProjection();
		static void UpdateView();

		static bool OnMouseScroll(MouseScrolledEvent& e);

		static void MousePan(const glm::vec2& delta);
		static void MouseRotate(const glm::vec2& delta);
		static void MouseZoom(float delta);

		static glm::vec3			   CalculatePosition();
		static std::pair<float, float> PanSpeed();
		static float				   ZoomSpeed();

	private:
		inline static glm::mat4 s_ViewMatrix;
		inline static glm::mat4 s_Projection;
		inline static glm::vec3 s_Position;
		inline static glm::vec3 s_FocalPoint;

		inline static glm::vec2 s_InitialMousePosition{};

		inline static float s_AspectRatio;
		inline static float s_FOV;
		inline static float s_NearClip;
		inline static float s_FarClip;

		inline static float s_Distance = 10.0f;
		inline static float s_Pitch = 0.0f;
		inline static float s_Yaw = 0.0f;

		inline static float s_ViewportWidth = 1280;
		inline static float s_ViewportHeight = 720;
	};
} // namespace askygg