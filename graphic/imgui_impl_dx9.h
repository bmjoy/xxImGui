// dear imgui: Renderer for DirectX9
// This needs to be used along with a Platform Binding (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'LPDIRECT3DTEXTURE9' as ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.
//  [X] Renderer: Multi-viewport support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable'.
//  [X] Renderer: Support for large meshes (64k+ vertices) with 16-bits indices.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

#pragma once

struct IDirect3DDevice9;

IMGUI_IMPL_API bool     ImGui_ImplDX9_Init(IDirect3DDevice9* device);
IMGUI_IMPL_API void     ImGui_ImplDX9_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplDX9_NewFrame();
IMGUI_IMPL_API void     ImGui_ImplDX9_RenderDrawData(ImDrawData* draw_data, void* dummy);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_IMPL_API bool     ImGui_ImplDX9_CreateDeviceObjects();
IMGUI_IMPL_API void     ImGui_ImplDX9_InvalidateDeviceObjects();

// Render flow
IMGUI_IMPL_API bool     ImGui_ImplDX9_Create(void* hWnd);
IMGUI_IMPL_API bool     ImGui_ImplDX9_Cleanup();
IMGUI_IMPL_API bool     ImGui_ImplDX9_Reset(void* hWnd = 0, int width = 0, int height = 0);
IMGUI_IMPL_API bool     ImGui_ImplDX9_Begin();
IMGUI_IMPL_API bool     ImGui_ImplDX9_Clear(const ImVec4& clear_color);
IMGUI_IMPL_API bool     ImGui_ImplDX9_Finish();
IMGUI_IMPL_API bool     ImGui_ImplDX9_Present();
