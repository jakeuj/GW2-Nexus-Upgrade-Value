#include <Windows.h>

#include <memory>

#include "App.h"
#include "imgui/imgui.h"
#include "nexus/Nexus.h"

namespace
{
    constexpr int AddonSignature = -26071501;
    constexpr const char* KeybindId = "KB_UPGRADE_VALUE_TOGGLE";
    constexpr const char* WindowName = "Upgrade Value##NexusUpgradeValue";

    HMODULE SelfModule = nullptr;
    AddonAPI* Api = nullptr;
    AddonDefinition Definition{};
    std::unique_ptr<UpgradeValue::App> Application;

    void Render()
    {
        if (Application) Application->Render();
    }

    void RenderOptions()
    {
        if (Application) Application->RenderOptions();
    }

    void ProcessKeybind(const char*, bool isRelease)
    {
        if (!isRelease && Application) Application->ToggleWindow();
    }

    void Load(AddonAPI* api)
    {
        Api = api;
        ImGui::SetCurrentContext(static_cast<ImGuiContext*>(Api->ImguiContext));
        ImGui::SetAllocatorFunctions(
            reinterpret_cast<void* (*)(size_t, void*)>(Api->ImguiMalloc),
            reinterpret_cast<void (*)(void*, void*)>(Api->ImguiFree));

        Application = std::make_unique<UpgradeValue::App>(Api);
        Application->Initialize();

        Api->Renderer.Register(ERenderType_Render, Render);
        Api->Renderer.Register(ERenderType_OptionsRender, RenderOptions);
        Api->InputBinds.RegisterWithString(KeybindId, ProcessKeybind, "ALT+SHIFT+U");
        Api->UI.RegisterCloseOnEscape(WindowName, Application->WindowVisiblePointer());
        Api->Log(ELogLevel_INFO, "UpgradeValue", "Upgrade Value loaded.");
    }

    void Unload()
    {
        Api->UI.DeregisterCloseOnEscape(WindowName);
        Api->InputBinds.Deregister(KeybindId);
        Api->Renderer.Deregister(RenderOptions);
        Api->Renderer.Deregister(Render);
        Application.reset();
        Api->Log(ELogLevel_INFO, "UpgradeValue", "Upgrade Value unloaded.");
        Api = nullptr;
    }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH) SelfModule = module;
    return TRUE;
}

extern "C" __declspec(dllexport) AddonDefinition* GetAddonDef()
{
    Definition.Signature = AddonSignature;
    Definition.APIVersion = NEXUS_API_VERSION;
    Definition.Name = "Upgrade Value";
    Definition.Version = {1, 0, 3, 0};
    Definition.Author = "jakeuj";
    Definition.Description = "Find embedded runes and sigils in Exotic gear and compare live Trading Post prices.";
    Definition.Load = Load;
    Definition.Unload = Unload;
    Definition.Flags = EAddonFlags_None;
    Definition.Provider = EUpdateProvider_GitHub;
    Definition.UpdateLink = "https://github.com/jakeuj/GW2-Nexus-Upgrade-Value";
    return &Definition;
}
