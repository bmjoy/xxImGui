//==============================================================================
// xxImGui : Plugin Source
//
// Copyright (c) 2019 TAiGA
// https://github.com/metarutaiga/xxImGui
//==============================================================================
#include "xxGraphic/xxSystem.h"
#include "DearImGui.h"
#include "Renderer.h"
#include "Plugin.h"

static ImVector<void*>                  g_pluginLibraries;
static ImVector<const char*>            g_pluginTitles;
static ImVector<PFN_PLUGIN_CREATE>      g_pluginCreates;
static ImVector<PFN_PLUGIN_SHUTDOWN>    g_pluginShutdowns;
static ImVector<PFN_PLUGIN_UPDATE>      g_pluginUpdates;
static ImVector<PFN_PLUGIN_RENDER>      g_pluginRenders;
//------------------------------------------------------------------------------
void Plugin::Create(const char* path)
{
    const char* app = xxGetExecutablePath();
    const char* configuration = "";
    const char* arch = "";
    const char* extension = "";
#if defined(_DEBUG)
    configuration = "debug";
#elif defined(NDEBUG)
    configuration = "release";
#endif
#if defined(_M_AMD64)
    arch = ".x64";
#elif defined(_M_IX86)
    arch = ".x86";
#endif
#if defined(xxWINDOWS)
    extension = ".dll";
#elif defined(xxMACOS)
    extension = ".dylib";
#elif defined(xxANDROID)
    extension = ".so";
#endif

    char temp[4096];
#if defined(xxWINDOWS)
    snprintf(temp, 4096, "%s/%s", app, path);
#elif defined(xxMACOS)
    snprintf(temp, 4096, "%s/../../..", app);
#elif defined(xxANDROID)
    snprintf(temp, 4096, "%s", app);
#endif

    uint64_t handle = 0;
    while (const char* filename = xxOpenDirectory(&handle, temp, configuration, arch, extension, nullptr))
    {
#if defined(xxWINDOWS)
        snprintf(temp, 4096, "%s/%s/%s", app, path, filename);
#elif defined(xxMACOS)
        snprintf(temp, 4096, "%s/../../../%s", app, filename);
#elif defined(xxANDROID)
        snprintf(temp, 4096, "%s/%s", app, filename);
#endif
        void* library = xxLoadLibrary(temp);
        if (library == nullptr)
            continue;

        PFN_PLUGIN_CREATE create = (PFN_PLUGIN_CREATE)xxGetProcAddress(library, "Create");
        PFN_PLUGIN_SHUTDOWN shutdown = (PFN_PLUGIN_SHUTDOWN)xxGetProcAddress(library, "Shutdown");
        PFN_PLUGIN_UPDATE update = (PFN_PLUGIN_UPDATE)xxGetProcAddress(library, "Update");
        PFN_PLUGIN_RENDER render = (PFN_PLUGIN_RENDER)xxGetProcAddress(library, "Render");
        if (create == nullptr || shutdown == nullptr || update == nullptr || render == nullptr)
        {
            xxFreeLibrary(library);
            continue;
        }
        xxLog("Plugin", "Loaded : %s", filename);

        g_pluginLibraries.push_back(library);
        g_pluginCreates.push_back(create);
        g_pluginShutdowns.push_back(shutdown);
        g_pluginUpdates.push_back(update);
        g_pluginRenders.push_back(render);
    }
    xxCloseDirectory(&handle);

    CreateData createData;
    createData.baseFolder = app;
    for (int i = 0; i < g_pluginCreates.size(); ++i)
    {
        PFN_PLUGIN_CREATE create = g_pluginCreates[i];
        const char* title = create(createData);
        g_pluginTitles.push_back(title);
    }
}
//------------------------------------------------------------------------------
void Plugin::Shutdown()
{
    ShutdownData shutdownData;
    for (int i = 0; i < g_pluginShutdowns.size(); ++i)
    {
        PFN_PLUGIN_SHUTDOWN shutdown = g_pluginShutdowns[i];
        shutdown(shutdownData);
    }

    for (int i = 0; i < g_pluginLibraries.size(); ++i)
    {
        void* library = g_pluginLibraries[i];
        xxFreeLibrary(library);
    }

    g_pluginLibraries.clear();
    g_pluginTitles.clear();
    g_pluginCreates.clear();
    g_pluginShutdowns.clear();
    g_pluginUpdates.clear();
    g_pluginRenders.clear();
}
//------------------------------------------------------------------------------
bool Plugin::Update()
{
    UpdateData updateData;
    updateData.device = Renderer::g_device;
    updateData.time = xxGetCurrentTime();
    updateData.windowScale = ImGui::GetStyle().MouseCursorScale;
    for (int i = 0; i < g_pluginUpdates.size(); ++i)
    {
        PFN_PLUGIN_UPDATE update = g_pluginUpdates[i];
        update(updateData);
    }

    return g_pluginUpdates.empty() == false;
}
//------------------------------------------------------------------------------
void Plugin::Render(uint64_t commandEncoder)
{
    RenderData renderData;
    renderData.device = Renderer::g_device;
    renderData.commandEncoder = commandEncoder;
    for (int i = 0; i < g_pluginRenders.size(); ++i)
    {
        PFN_PLUGIN_RENDER render = g_pluginRenders[i];
        render(renderData);
    }
}
//------------------------------------------------------------------------------
