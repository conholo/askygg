#include "askygg/core/Application.h"
#include "askygg/platform/renderer_platform/metal/MetalWindow.h"

namespace askygg
{
	MetalWindow::MetalWindow(const WindowProperties& windowProperties)
		: m_Frame((CGRect){ { 100.0, 100.0 }, { 512.0, 512.0 } })
	{
		m_NativeWindowHandle =
			NS::Window::alloc()->init(m_Frame, NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
				NS::BackingStoreBuffered, false);

		m_NativeWindowHandle->setTitle(
			NS::String::string(windowProperties.Title.c_str(), NS::StringEncoding::UTF8StringEncoding));
		m_NativeWindowHandle->makeKeyAndOrderFront(nullptr);
	}

	MetalWindow::~MetalWindow() {}

	NS::Menu* MetalWindow::CreateMenuBar()
	{
		using NS::StringEncoding::UTF8StringEncoding;

		NS::Menu*	  mainMenu = NS::Menu::alloc()->init();
		NS::MenuItem* appMenuItem = NS::MenuItem::alloc()->init();
		NS::Menu*	  appMenu = NS::Menu::alloc()->init(
				NS::String::string(Application::GetApplication().GetName().c_str(), UTF8StringEncoding));

		NS::String* appName = NS::RunningApplication::currentApplication()->localizedName();
		NS::String* quitItemName =
			NS::String::string("Quit ", UTF8StringEncoding)->stringByAppendingString(appName);
		SEL quitCb =
			NS::MenuItem::registerActionCallback("appQuit", [](void*, SEL, const NS::Object* pSender) {
				auto pApp = NS::Application::sharedApplication();
				pApp->terminate(pSender);
			});

		NS::MenuItem* appQuitItem =
			appMenu->addItem(quitItemName, quitCb, NS::String::string("q", UTF8StringEncoding));
		appQuitItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);
		appMenuItem->setSubmenu(appMenu);

		NS::MenuItem* windowMenuItem = NS::MenuItem::alloc()->init();
		NS::Menu*	  windowMenu = NS::Menu::alloc()->init(NS::String::string("Window", UTF8StringEncoding));

		SEL closeWindowCb =
			NS::MenuItem::registerActionCallback("windowClose", [](void*, SEL, const NS::Object*) {
				auto pApp = NS::Application::sharedApplication();
				pApp->windows()->object<NS::Window>(0)->close();
			});
		NS::MenuItem* closeWindowItem =
			windowMenu->addItem(NS::String::string("Close Window", UTF8StringEncoding), closeWindowCb,
				NS::String::string("w", UTF8StringEncoding));
		closeWindowItem->setKeyEquivalentModifierMask(NS::EventModifierFlagCommand);

		windowMenuItem->setSubmenu(windowMenu);

		mainMenu->addItem(appMenuItem);
		mainMenu->addItem(windowMenuItem);

		appMenuItem->release();
		windowMenuItem->release();
		appMenu->release();
		windowMenu->release();

		return mainMenu->autorelease();
	}

	void MetalWindow::OnUpdate() {}
} // namespace askygg