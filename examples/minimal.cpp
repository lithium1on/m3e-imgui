#include <m3e/imgui.hpp>

int main(int argc, char** argv) {
    // In an application, use its existing context and platform/renderer NewFrame calls.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.DisplaySize = {640, 360};
    io.DeltaTime = 1.0f / 60;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;
    m3e::Style material;
    if (argc > 1)
        material.fonts = m3e::LoadFonts(argv[1]);

    bool notify = true;
    bool saved = false;
    char name[80] = "";
    for (int frame = 0; frame < 2; ++frame) {
        ImGui::NewFrame();
        ImGui::SetNextWindowSize({360, 240});
        ImGui::Begin("Settings");
        m3e::TextField(material, "Display name", name, sizeof(name));
        m3e::Switch(material, "Notifications", notify, m3e::SwitchIcons::Selected);
        m3e::ButtonOptions save;
        save.variant = m3e::Variant::Filled;
        save.icon = m3e::icons::Check;
        saved |= m3e::Button(material, "Save", save);
        if (saved)
            ImGui::TextUnformatted("Saved");
        ImGui::End();
        ImGui::Render();
        // Pass ImGui::GetDrawData() to your renderer backend here.
    }
    const bool emitted = ImGui::GetDrawData()->TotalVtxCount > 0;
    ImGui::DestroyContext();
    return emitted ? 0 : 1;
}
