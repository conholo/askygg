#pragma once

#include <glm/glm.hpp>
#include "askygg/renderer/BufferLayout.h"
#include "askygg/renderer/Material.h"

namespace askygg::UI
{
	enum class UIPropertyType
	{
		None = 0,
		Float,
		Vec2,
		Vec3,
		Vec4,
		Color,
		Texture,
		Int,
		Bool
	};

	UIPropertyType UIPropertyTypeFromShaderDataType(ShaderAttributeType ShaderAttributeType,
		bool															isColor = true);

	struct UIFloatParameters
	{
		UIFloatParameters() = default;

		UIFloatParameters(const UIFloatParameters&) = default;

		bool		IsDrag = true;
		float		Min = 0.0f;
		float		Max = 1.0f;
		float		SpeedStep = 0.01f;
		float		FastStep = 0.0f;
		const char* Format = "%.3f";
	};

	class UIProperty
	{
	public:
		UIProperty() = default;

		UIProperty(const std::string& label)
			: m_Label(label), m_UUID(s_UIDCounter++) {}

		virtual ~UIProperty() = default;

		virtual void Draw() = 0;

		const std::string& GetLabel() const { return m_Label; }

	protected:
		std::string		m_Label;
		uint32_t		m_UUID;
		static uint32_t s_UIDCounter;
	};

	class UIFloat : public UIProperty
	{
	public:
		UIFloat() = default;

		UIFloat(const std::string& label, float* value)
			: UIProperty(label), m_Value(value)
		{
			if (*value > m_FloatParameters.Max)
				m_FloatParameters.Max = *value;
		}

		void Draw() override;

		static void Draw(const std::string& label, float* value);

		static bool DrawSlider(const std::string& label, float* value, float min, float max);

		static bool DrawAngle(const std::string& label, float* radians, float min, float max);

		void SetIsDrag(bool isDrag) { m_FloatParameters.IsDrag = isDrag; }

		void SetMin(float min) { m_FloatParameters.Min = min; }

		void SetMax(float max) { m_FloatParameters.Max = max; }

		void SetSpeedStep(float speedStep) { m_FloatParameters.SpeedStep = speedStep; }

	private:
		UIFloatParameters m_FloatParameters;
		float			  m_DefaultValue = 0.0f;
		float*			  m_Value = nullptr;
	};

	class UIInt : public UIProperty
	{
	public:
		UIInt() = default;
		UIInt(const std::string& label, int* value)
			: UIProperty(label), m_Value(value) {}

		void Draw(const std::string& label, int* value);
		static void DrawDragInt(const std::string& label, int* value, float speed, int min, int max);

	private:
		int	 m_Step = 1;
		int* m_Value = nullptr;

		float m_DefaultValue = 0.0;
	};

	class UIVector : public UIProperty
	{
	public:
		UIVector() = default;

		UIVector(const std::string& label)
			: UIProperty(label)
		{
			m_FloatParameters.Min = 0.0f;
			m_FloatParameters.Max = 0.0f;
		}

		virtual void Draw() = 0;

		void SetIsDrag(bool isDrag) { m_FloatParameters.IsDrag = isDrag; }

		void SetMin(float min) { m_FloatParameters.Min = min; }

		void SetMax(float max) { m_FloatParameters.Max = max; }

		void SetSpeedStep(float speedStep) { m_FloatParameters.SpeedStep = speedStep; }

	protected:
		UIFloatParameters m_FloatParameters;
	};

	class UIVector2 : public UIVector
	{
	public:
		UIVector2() = default;

		UIVector2(const std::string& label, glm::vec2* value)
			: UIVector(label), m_Value(value) {}

		void Draw() override;

	private:
		glm::vec2* m_Value = nullptr;
		glm::vec2  m_DefaultValue{ 0.0f };
	};

	class UIVector3 : public UIVector
	{
	public:
		UIVector3() = default;

		UIVector3(const std::string& label, glm::vec3* value)
			: UIVector(label), m_Value(value) {}

		void Draw() override;

		static void Draw(const std::string& label, glm::vec3* value, bool readonly = false);

	private:
		glm::vec3* m_Value = nullptr;
		glm::vec3  m_DefaultValue{ 0.0f };
	};

	class UIVector4 : public UIVector
	{
	public:
		UIVector4() = default;

		UIVector4(const std::string& label, glm::vec4* value)
			: UIVector(label), m_Value(value) {}

		void Draw() override;

	private:
		glm::vec4* m_Value = nullptr;
		glm::vec4  m_DefaultValue{ 0.0f };
	};

	class UIColor : UIProperty
	{
	public:
		UIColor() = default;

		UIColor(const std::string& label, glm::vec4* color)
			: UIProperty(label), m_Color(color), m_ColorVec4Drawer("", color) {}

		void Draw() override;

	private:
		UIVector4  m_ColorVec4Drawer;
		glm::vec4* m_Color = nullptr;
		glm::vec4  m_DefaultColor{ 1.0f };
	};

	class UITexture2D : UIProperty
	{
	public:
		UITexture2D() = default;

		UITexture2D(const std::string& label, const Ref<Material>& material, TextureUniform* value,
			const std::string& textureUniformName)
			: UIProperty(label), m_TextureUniformName(textureUniformName), m_TextureUniform(value), m_Material(material) {}

		void Draw() override;

	private:
		std::string		m_TextureUniformName;
		TextureUniform* m_TextureUniform{};
		Ref<Material>	m_Material{};
		glm::vec4		m_DefaultColor{ 1.0f };
	};

	class UIBool : UIProperty
	{
	public:
		UIBool() = default;

		UIBool(const std::string& label, bool* value)
			: UIProperty(label), m_Value(value) {}

		void Draw() override;

		static bool Draw(const std::string& label, bool* value);

	private:
		bool* m_Value;
	};
} // namespace askygg::UI
