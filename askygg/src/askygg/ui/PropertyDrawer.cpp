#include "askygg/ui/PropertyDrawer.h"
#include "askygg/core/Input.h"
#include "askygg/core/Log.h"
#include "askygg/renderer/Texture.h"
#include "askygg/ui/UIDrawerHelpers.h"

#include <imgui.h>
#include "imgui_internal.h"

#include <sstream>

namespace askygg::UI
{
	uint32_t UIProperty::s_UIDCounter = 0;

	UIPropertyType UIPropertyTypeFromShaderDataType(ShaderAttributeType ShaderDataType, bool isColor)
	{
		switch (ShaderDataType)
		{
			case ShaderAttributeType::Float:
				return UIPropertyType::Float;
			case ShaderAttributeType::Float2:
				return UIPropertyType::Vec2;
			case ShaderAttributeType::Float3:
				return UIPropertyType::Vec3;
			case ShaderAttributeType::Float4:
				return isColor ? UIPropertyType::Color : UIPropertyType::Vec4;
			case ShaderAttributeType::Int:
				return UIPropertyType::Int;
			case ShaderAttributeType::Sampler2D:
				return UIPropertyType::Texture;
			default:
				return UIPropertyType::None;
		}
	}

	static void ClampFloat(float* value, float min, float max)
	{
		*value = *value < min ? min : (*value > max ? max : *value);
	}

	static void DrawFloatParametersPopup(uint32_t uuid, UIFloatParameters& floatParams)
	{
		std::stringstream ss;
		ss << "Float Properties"
		   << "##" << uuid;

		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0) && Input::IsKeyPressed(Key::LeftShift))
			ImGui::OpenPopup(ss.str().c_str());

		if (ImGui::BeginPopup(ss.str().c_str()))
		{
			std::stringstream ssDrag;
			ssDrag << "Drag##" << uuid;
			ImGui::Checkbox(ssDrag.str().c_str(), &floatParams.IsDrag);

			std::stringstream ssMinValue;
			ssMinValue << "Min Value##" << uuid;
			ImGui::InputFloat(ssMinValue.str().c_str(), &floatParams.Min);

			std::stringstream ssMaxValue;
			ssMaxValue << "Max Value##" << uuid;
			ImGui::InputFloat(ssMaxValue.str().c_str(), &floatParams.Max);

			std::stringstream ssStep;
			ssStep << "Step##" << uuid;
			ImGui::InputFloat(ssStep.str().c_str(), &floatParams.SpeedStep);

			ImGui::EndPopup();
		}
	}

	void UIFloat::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		std::stringstream ss;
		ss << "Float Properties"
		   << "##" << m_UUID;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable(m_Label.c_str(), 2,
				ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", m_Label.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			if (m_FloatParameters.IsDrag)
				ImGui::DragFloat("", m_Value, m_FloatParameters.SpeedStep, m_FloatParameters.Min,
					m_FloatParameters.Max, m_FloatParameters.Format);
			else
				ImGui::InputFloat("", m_Value, m_FloatParameters.SpeedStep, m_FloatParameters.FastStep,
					m_FloatParameters.Format);
			ImGui::PopItemWidth();
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		ClampFloat(m_Value, m_FloatParameters.Min, m_FloatParameters.Max);

		ImGui::PopID();
	}

	void UIFloat::Draw(const std::string& label, float* value)
	{
		ImGui::PushID(label.c_str());

		std::stringstream ss;
		ss << "Float Properties"
		   << "##" << label;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable(label.c_str(), 2,
				ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", label.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			ImGui::DragFloat("", value, 0.01f, 0.0f, 0.0f, "%.3f");
			ImGui::PopItemWidth();
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		ImGui::PopID();
	}

	bool UIFloat::DrawSlider(const std::string& label, float* value, float min, float max)
	{
		bool updated = false;
		ImGui::PushID(label.c_str());

		std::stringstream ss;
		ss << "Float Properties"
		   << "##" << label;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable(label.c_str(), 2,
				ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", label.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			updated |= ImGui::SliderFloat("", value, min, max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::PopItemWidth();
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		ImGui::PopID();

		return updated;
	}

	bool UIFloat::DrawAngle(const std::string& label, float* radians, float min, float max)
	{
		bool updated = false;
		ImGui::PushID(label.c_str());

		std::stringstream ss;
		ss << "Float Properties"
		   << "##" << label;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable(label.c_str(), 2,
				ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", label.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			updated |= ImGui::SliderAngle("", radians, min, max, "%.3f", ImGuiSliderFlags_AlwaysClamp);
			ImGui::PopItemWidth();
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		ImGui::PopID();

		return updated;
	}

	void UIInt::DrawDragInt(const std::string& label, int* value, float speed, int min, int max)
	{
		ImGui::DragInt(label.c_str(), value, speed, min, max);
	}

    void UIVector2::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		std::stringstream ss;
		ss << "Vector2 Properties"
		   << "##" << m_UUID;

		if (m_FloatParameters.IsDrag)
		{
			std::stringstream ssDragFloat;
			ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

			ImGui::DragFloat2(ssDragFloat.str().c_str(), &m_Value->x, m_FloatParameters.SpeedStep,
				m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}
		else
		{
			std::stringstream ssInputFloat;
			ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

			ImGui::InputFloat2(ssInputFloat.str().c_str(), &m_Value->x, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}

		if (m_FloatParameters.Min < m_FloatParameters.Max && m_FloatParameters.Min != 0 && m_FloatParameters.Max != 0)
		{
			ClampFloat(&m_Value->x, m_FloatParameters.Min, m_FloatParameters.Max);
			ClampFloat(&m_Value->y, m_FloatParameters.Min, m_FloatParameters.Max);
		}

		ImGui::PopID();
	}

	void UIVector3::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		std::stringstream ss;
		ss << "Vector3 Properties"
		   << "##" << m_UUID;

		if (m_FloatParameters.IsDrag)
		{
			std::stringstream ssDragFloat;
			ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

			ImGui::DragFloat3(ssDragFloat.str().c_str(), &m_Value->x, m_FloatParameters.SpeedStep,
				m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}
		else
		{
			std::stringstream ssInputFloat;
			ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

			ImGui::InputFloat3(ssInputFloat.str().c_str(), &m_Value->x, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}

		if (m_FloatParameters.Min < m_FloatParameters.Max && m_FloatParameters.Min != 0 && m_FloatParameters.Max != 0)
		{
			ClampFloat(&m_Value->x, m_FloatParameters.Min, m_FloatParameters.Max);
			ClampFloat(&m_Value->y, m_FloatParameters.Min, m_FloatParameters.Max);
			ClampFloat(&m_Value->z, m_FloatParameters.Min, m_FloatParameters.Max);
		}

		ImGui::PopID();
	}

	void UIVector3::Draw(const std::string& label, glm::vec3* value, bool readonly)
	{
		ImGui::PushID(label.c_str());

		if (readonly)
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);

		std::stringstream ss;
		ss << "Float Properties"
		   << "##" << label;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable(label.c_str(), 2,
				ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", label.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
			DrawVector3FieldTable("", *value, 1.0f);
			ImGui::PopItemWidth();
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		if (readonly)
			ImGui::PopItemFlag();

		ImGui::PopID();
	}

	void UIVector4::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		std::stringstream ss;
		ss << "Vector4 Properties"
		   << "##" << m_UUID;

		if (m_FloatParameters.IsDrag)
		{
			std::stringstream ssDragFloat;
			ssDragFloat << m_Label.c_str() << "##Drag" << m_UUID;

			// ImGui::DragFloat4(ssDragFloat.str().c_str(), &m_Value->x, m_FloatParameters.SpeedStep,
			// m_FloatParameters.Min, m_FloatParameters.Max, m_FloatParameters.Format);
			DrawVector4Field2("", *m_Value, 1.0f);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}
		else
		{
			std::stringstream ssInputFloat;
			ssInputFloat << m_Label.c_str() << "##Input" << m_UUID;

			ImGui::InputFloat4(ssInputFloat.str().c_str(), &m_Value->x, m_FloatParameters.Format);
			DrawFloatParametersPopup(m_UUID, m_FloatParameters);
		}

		if (m_FloatParameters.Min < m_FloatParameters.Max && m_FloatParameters.Min != 0 && m_FloatParameters.Max != 0)
		{
			ClampFloat(&m_Value->x, m_FloatParameters.Min, m_FloatParameters.Max);
			ClampFloat(&m_Value->y, m_FloatParameters.Min, m_FloatParameters.Max);
			ClampFloat(&m_Value->z, m_FloatParameters.Min, m_FloatParameters.Max);
			ClampFloat(&m_Value->w, m_FloatParameters.Min, m_FloatParameters.Max);
		}

		ImGui::PopID();
	}

	void UIColor::Draw()
	{
		ImGui::PushID(m_Label.c_str());

		ImVec4			  color{ m_Color->r, m_Color->g, m_Color->b, m_Color->a };
		std::stringstream pickerSS;
		pickerSS << "\"" << m_Label.c_str() << "\""
				 << " Picker";

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable(m_Label.c_str(), 2,
				ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", m_Label.c_str());

			ImGui::TableSetColumnIndex(1);
			if (ImGui::ColorButton(pickerSS.str().c_str(), color, ImGuiColorEditFlags_HDR, { 10, 10 }))
				ImGui::OpenPopup("Color Picker");

			if (ImGui::BeginPopup("Color Picker"))
			{
				ImGuiColorEditFlags flags = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float;
				ImGui::ColorPicker4(m_Label.c_str(), &m_Color->x, flags, nullptr);
				ImGui::EndPopup();
			}

			m_ColorVec4Drawer.Draw();

			ImGui::EndTable();
		}

		ImGui::PopStyleVar();
		ImGui::PopID();
	}

	void UITexture2D::Draw()
	{
		if (m_TextureUniform->HideInUI)
			return;

		if (TextureLibrary::Has2DFromID(m_TextureUniform->RendererID))
			return;

		Texture2D& CurrentTexture = TextureLibrary::Get2DFromID(m_TextureUniform->RendererID);

		ImGui::PushID(m_Label.c_str());

		std::stringstream ss;
		ss << "##" << m_UUID;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable(m_Label.c_str(), 2,
				ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::LabelText(ss.str().c_str(), "%s", m_Label.c_str());

			ImGui::TableSetColumnIndex(1);
			if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(CurrentTexture.GetID()), { 50, 50 },
					{ 0, 1 }, { 1, 0 }))
				ImGui::OpenPopup("Texture Selection");

			if (ImGui::BeginPopup("Texture Selection"))
			{
				if (ImGui::BeginCombo("Texture Library", CurrentTexture.GetName().c_str()))
				{
					std::unordered_map<std::string, Ref<Texture2D>> availableTextures =
						TextureLibrary::Get2DLibrary();

					for (auto [name, texture] : availableTextures)
					{
						if (texture->GetID() == m_TextureUniform->RendererID)
							continue;
						if (ImGui::Selectable(name.c_str(), true))
							m_TextureUniform->RendererID = texture->GetID();
					}

					ImGui::EndCombo();
				}

				ImGui::EndPopup();
			}

			ImGui::EndTable();
		}

		ImGui::PopStyleVar();
		ImGui::PopID();
	}

	void UIBool::Draw()
	{
		ImGui::Checkbox(m_Label.c_str(), m_Value);
	}

	bool UIBool::Draw(const std::string& label, bool* value)
	{
		bool Updated = false;
		ImGui::PushID(label.c_str());

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		if (ImGui::BeginTable(label.c_str(), 2,
				ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::AlignTextToFramePadding();
			ImGui::Text("%s", label.c_str());
			ImGui::TableSetColumnIndex(1);
			Updated |= ImGui::Checkbox("", value);
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		ImGui::PopID();

		return Updated;
	}
} // namespace askygg::UI
