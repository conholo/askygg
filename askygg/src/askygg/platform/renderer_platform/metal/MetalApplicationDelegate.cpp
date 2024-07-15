#include "askygg/platform/renderer_platform/metal/MetalApplicationDelegate.h"

namespace askygg
{
	MetalApplicationDelegate::MetalApplicationDelegate(const Ref<MetalWindow>& window,
		const Ref<MetalDevice>&												   device,
		const Ref<MetalViewDelegate>&										   viewDelegate)
		: m_WindowRef(window), m_DeviceRef(device), m_ViewDelegateRef(viewDelegate) {}

	MetalApplicationDelegate::~MetalApplicationDelegate() {}

	void MetalApplicationDelegate::applicationWillFinishLaunching(NS::Notification* notification)
	{
		NS::Menu* menu = m_WindowRef->CreateMenuBar();
		auto*	  app = reinterpret_cast<NS::Application*>(notification->object());
		app->setMainMenu(menu);
		app->setActivationPolicy(NS::ActivationPolicy::ActivationPolicyRegular);
	}

	void MetalApplicationDelegate::applicationDidFinishLaunching(NS::Notification* notification)
	{
		m_MTKView = MTK::View::alloc()->init(m_WindowRef->GetFrame(), &m_DeviceRef->GetDevice());
		m_MTKView->setColorPixelFormat(MTL::PixelFormat::PixelFormatBGRA8Unorm);
		m_MTKView->setClearColor(MTL::ClearColor::Make(1.0, 0.0, 0.0, 1.0));
		m_MTKView->setDelegate(&(*m_ViewDelegateRef));

		auto* window = static_cast<NS::Window*>(m_WindowRef->GetNativeWindow());
		window->setContentView(m_MTKView);

		auto* app = reinterpret_cast<NS::Application*>(notification->object());
		app->activateIgnoringOtherApps(true);
	}
} // namespace askygg