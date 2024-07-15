#pragma once

#include "askygg/core/Application.h"
#include "askygg/core/Memory.h"

#include "Metal/Metal.hpp"
#include "AppKit/AppKit.hpp"
#include "MetalKit/MetalKit.hpp"

#include "MetalWindow.h"
#include "MetalRenderer.h"
#include "MetalViewDelegate.h"

namespace askygg
{
	class MetalApplicationDelegate : public NS::ApplicationDelegate
	{
	public:
		explicit MetalApplicationDelegate(const Ref<MetalWindow>& window, const Ref<MetalDevice>& device,
			const Ref<MetalViewDelegate>& viewDelegate);

		~MetalApplicationDelegate() override;

		void applicationWillFinishLaunching(NS::Notification* notification) override;
		void applicationDidFinishLaunching(NS::Notification* notification) override;
		bool applicationShouldTerminateAfterLastWindowClosed(NS::Application* sender) override
		{
			return true;
		}

		MetalWindow& GetWindow() { return *m_WindowRef; }
		MetalDevice& GetDevice() { return *m_DeviceRef; }

	private:
		Ref<MetalWindow>	   m_WindowRef;
		Ref<MetalDevice>	   m_DeviceRef;
		Ref<MetalViewDelegate> m_ViewDelegateRef;

		MTK::View* m_MTKView = nullptr;
	};
} // namespace askygg