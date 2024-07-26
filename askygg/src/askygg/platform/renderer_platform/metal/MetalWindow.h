#pragma once


#include "askygg/core/Window.h"

#include "Metal/Metal.hpp"
#include "AppKit/AppKit.hpp"


namespace askygg
{
    class MetalWindow : public Window
    {
    public:
        explicit MetalWindow(const WindowProperties& windowProperties = WindowProperties());
        ~MetalWindow() override;

        void OnUpdate() override;
        void SetEventCallback(const EventCallbackFn &callback) override { }
        void ToggleIsMaximized(bool maximize) const override { }
        void SetVSync(bool enabled) override { }

        [[nodiscard]] virtual bool IsVSync() const override { return false; }
        [[nodiscard]] virtual void* GetNativeWindow() const override { return m_NativeWindowHandle; }
        [[nodiscard]] virtual uint32_t GetWidth() const override { return m_Frame.size.width; }
        [[nodiscard]] virtual uint32_t GetHeight() const override { return m_Frame.size.height; }


        NS::Menu* CreateMenuBar();
        [[nodiscard]] CGRect GetFrame() const { return m_Frame; }

    private:
        CGRect m_Frame;
        NS::Window* m_NativeWindowHandle = nullptr;
    };
}