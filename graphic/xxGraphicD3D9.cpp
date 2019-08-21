#include "xxGraphicD3D9.h"
#include "xxGraphicD3DAsm.h"
#include "xxGraphicInternal.h"

#include "dxsdk/d3d9.h"
typedef LPDIRECT3D9 (WINAPI *PFN_DIRECT3D_CREATE9)(UINT);

static const wchar_t* const         g_dummy = L"xxGraphicDummyWindow";
static HMODULE                      g_d3dLibrary = nullptr;
static HWND                         g_hWnd = nullptr;
static LPDIRECT3DSURFACE9           g_depthStencil = nullptr;

//==============================================================================
//  Resource Type
//==============================================================================
static uint64_t getResourceType(uint64_t resource)
{
    return resource & 7;
}
//------------------------------------------------------------------------------
static uint64_t getResourceData(uint64_t resource)
{
    return resource & -8;
}
//==============================================================================
//  Instance
//==============================================================================
uint64_t xxCreateInstanceD3D9()
{
    if (g_d3dLibrary == nullptr)
        g_d3dLibrary = LoadLibraryW(L"d3d9.dll");
    if (g_d3dLibrary == nullptr)
        return 0;

    PFN_DIRECT3D_CREATE9 Direct3DCreate9;
    (void*&)Direct3DCreate9 = GetProcAddress(g_d3dLibrary, "Direct3DCreate9");
    if (Direct3DCreate9 == nullptr)
        return 0;

    LPDIRECT3D9 d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d == nullptr)
        return 0;

    xxRegisterFunction(D3D9);

    return reinterpret_cast<uint64_t>(d3d);
}
//------------------------------------------------------------------------------
void xxDestroyInstanceD3D9(uint64_t instance)
{
    LPDIRECT3D9 d3d = reinterpret_cast<LPDIRECT3D9>(instance);
    if (d3d == nullptr)
        return;
    d3d->Release();

    if (g_d3dLibrary)
    {
        FreeLibrary(g_d3dLibrary);
        g_d3dLibrary = nullptr;
    }

    xxUnregisterFunction();
}
//==============================================================================
//  Device
//==============================================================================
uint64_t xxCreateDeviceD3D9(uint64_t instance)
{
    LPDIRECT3D9 d3d = reinterpret_cast<LPDIRECT3D9>(instance);
    if (d3d == nullptr)
        return 0;

    if (g_hWnd == nullptr)
    {
        WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), CS_CLASSDC, DefWindowProcW, 0, 0, GetModuleHandleW(nullptr), nullptr, nullptr, nullptr, nullptr, g_dummy, nullptr };
        RegisterClassExW(&wc);
        g_hWnd = CreateWindowW(g_dummy, g_dummy, WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, 1, 1, nullptr, nullptr, wc.hInstance, nullptr);
    }

    D3DPRESENT_PARAMETERS d3dPresentParameters = {};
    d3dPresentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dPresentParameters.hDeviceWindow = g_hWnd;
    d3dPresentParameters.Windowed = TRUE;
    d3dPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    LPDIRECT3DDEVICE9 d3dDevice = nullptr;
    HRESULT hResult = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, nullptr, D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dPresentParameters, &d3dDevice);
    if (hResult != S_OK)
        return 0;

    return reinterpret_cast<uint64_t>(d3dDevice);
}
//------------------------------------------------------------------------------
void xxDestroyDeviceD3D9(uint64_t device)
{
    if (g_depthStencil)
    {
        g_depthStencil->Release();
        g_depthStencil = nullptr;
    }

    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(device);
    if (d3dDevice == nullptr)
        return;
    d3dDevice->Release();

    if (g_hWnd)
    {
        DestroyWindow(g_hWnd);
        UnregisterClassW(g_dummy, GetModuleHandleW(nullptr));
        g_hWnd = nullptr;
    }
}
//------------------------------------------------------------------------------
void xxResetDeviceD3D9(uint64_t device)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(device);
    if (d3dDevice == nullptr)
        return;

    if (g_depthStencil)
        g_depthStencil->Release();
    g_depthStencil = nullptr;

    D3DPRESENT_PARAMETERS d3dPresentParameters = {};
    d3dPresentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dPresentParameters.hDeviceWindow = g_hWnd;
    d3dPresentParameters.Windowed = TRUE;
    d3dPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    d3dDevice->Reset(&d3dPresentParameters);
}
//------------------------------------------------------------------------------
bool xxTestDeviceD3D9(uint64_t device)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(device);
    if (d3dDevice == nullptr)
        return false;

    HRESULT hResult = d3dDevice->TestCooperativeLevel();
    return hResult == S_OK;
}
//------------------------------------------------------------------------------
const char* xxGetDeviceStringD3D9(uint64_t device)
{
    return "Direct3D 9.0 Fixed-Function";
}
//==============================================================================
//  Swapchain
//==============================================================================
uint64_t xxCreateSwapchainD3D9(uint64_t device, void* view, unsigned int width, unsigned int height)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(device);
    if (d3dDevice == nullptr)
        return 0;

    HWND hWnd = (HWND)view;
    if (width == 0 || height == 0)
    {
        RECT rect = {};
        if (GetClientRect(hWnd, &rect) == TRUE)
        {
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
        }
    }

    D3DPRESENT_PARAMETERS d3dPresentParameters = {};
    d3dPresentParameters.BackBufferWidth = width;
    d3dPresentParameters.BackBufferHeight = height;
    d3dPresentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dPresentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dPresentParameters.hDeviceWindow = hWnd;
    d3dPresentParameters.Windowed = TRUE;
    d3dPresentParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    LPDIRECT3DSWAPCHAIN9 d3dSwapchain = nullptr;
    HRESULT hResult = d3dDevice->CreateAdditionalSwapChain(&d3dPresentParameters, &d3dSwapchain);
    if (hResult != S_OK)
        return 0;

    D3DSURFACE_DESC desc = {};
    if (g_depthStencil)
        g_depthStencil->GetDesc(&desc);
    if (desc.Width < width || desc.Height < height)
    {
        if (desc.Width < width)
            desc.Width = width;
        if (desc.Height < height)
            desc.Height = height;
        if (g_depthStencil)
            g_depthStencil->Release();
        d3dDevice->CreateDepthStencilSurface(desc.Width, desc.Height, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, FALSE, &g_depthStencil, nullptr);
    }

    return reinterpret_cast<uint64_t>(d3dSwapchain);
}
//------------------------------------------------------------------------------
void xxDestroySwapchainD3D9(uint64_t swapchain)
{
    LPDIRECT3DSWAPCHAIN9 d3dSwapchain = reinterpret_cast<LPDIRECT3DSWAPCHAIN9>(swapchain);
    if (d3dSwapchain == nullptr)
        return;
    d3dSwapchain->Release();
}
//------------------------------------------------------------------------------
void xxPresentSwapchainD3D9(uint64_t swapchain, void* view)
{
    LPDIRECT3DSWAPCHAIN9 d3dSwapchain = reinterpret_cast<LPDIRECT3DSWAPCHAIN9>(swapchain);
    if (d3dSwapchain == nullptr)
        return;
    d3dSwapchain->Present(nullptr, nullptr, (HWND)view, nullptr, 0);
}
//==============================================================================
//  Command Buffer
//==============================================================================
uint64_t xxGetCommandBufferD3D9(uint64_t device, uint64_t swapchain)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(device);
    if (d3dDevice == nullptr)
        return 0;

    LPDIRECT3DSWAPCHAIN9 d3dSwapchain = reinterpret_cast<LPDIRECT3DSWAPCHAIN9>(swapchain);
    if (d3dSwapchain == nullptr)
        return 0;

    LPDIRECT3DSURFACE9 surface = nullptr;
    d3dSwapchain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &surface);
    d3dDevice->SetRenderTarget(0, surface);
    d3dDevice->SetDepthStencilSurface(g_depthStencil);
    if (surface)
        surface->Release();

    return device;
}
//------------------------------------------------------------------------------
bool xxBeginCommandBufferD3D9(uint64_t commandBuffer)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return false;
    HRESULT hResult = d3dDevice->BeginScene();
    if (hResult != S_OK)
        return false;

    d3dDevice->SetPixelShader(nullptr);
    d3dDevice->SetVertexShader(nullptr);
    d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    d3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    d3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    d3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    d3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    d3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    d3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    d3dDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    d3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    d3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    d3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    return true;
}
//------------------------------------------------------------------------------
void xxEndCommandBufferD3D9(uint64_t commandBuffer)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;
    d3dDevice->EndScene();
}
//------------------------------------------------------------------------------
void xxSubmitCommandBufferD3D9(uint64_t commandBuffer)
{
}
//==============================================================================
//  Render Pass
//==============================================================================
union D3DRENDERPASS9
{
    uint64_t value;
    struct
    {
        D3DCOLOR color;
        uint16_t depth;
        uint8_t stencil;
        uint8_t flags;
    };
};
//------------------------------------------------------------------------------
uint64_t xxCreateRenderPassD3D9(uint64_t device, float r, float g, float b, float a, float depth, unsigned char stencil)
{
    D3DRENDERPASS9 d3dRenderPass;

    d3dRenderPass.color = D3DCOLOR_COLORVALUE(r, g, b, a);
    d3dRenderPass.depth = (uint16_t)(depth * UINT16_MAX);
    d3dRenderPass.stencil = stencil;
    d3dRenderPass.flags = D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL;

    return d3dRenderPass.value;
}
//------------------------------------------------------------------------------
void xxDestroyRenderPassD3D9(uint64_t renderPass)
{

}
//------------------------------------------------------------------------------
bool xxBeginRenderPassD3D9(uint64_t commandBuffer, uint64_t renderPass)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return false;
    D3DRENDERPASS9 d3dRenderPass = { renderPass };

    float depth = d3dRenderPass.depth / (float)UINT16_MAX;
    HRESULT hResult = d3dDevice->Clear(0, nullptr, d3dRenderPass.flags, d3dRenderPass.color, depth, d3dRenderPass.stencil);
    return hResult == S_OK;
}
//------------------------------------------------------------------------------
void xxEndRenderPassD3D9(uint64_t commandBuffer, uint64_t renderPass)
{

}
//==============================================================================
//  Buffer
//==============================================================================
uint64_t xxCreateConstantBufferD3D9(uint64_t device, unsigned int size)
{
    char* d3dBuffer = xxAlloc(char, size);
    return reinterpret_cast<uint64_t>(d3dBuffer);
}
//------------------------------------------------------------------------------
uint64_t xxCreateIndexBufferD3D9(uint64_t device, unsigned int size)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(device);
    if (d3dDevice == nullptr)
        return 0;

    LPDIRECT3DINDEXBUFFER9 d3dIndexBuffer = nullptr;
    HRESULT hResult = d3dDevice->CreateIndexBuffer(size, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &d3dIndexBuffer, nullptr);
    if (hResult != S_OK)
        return 0;
    return reinterpret_cast<uint64_t>(d3dIndexBuffer) | D3DRTYPE_INDEXBUFFER;
}
//------------------------------------------------------------------------------
uint64_t xxCreateVertexBufferD3D9(uint64_t device, unsigned int size)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(device);
    if (d3dDevice == nullptr)
        return 0;

    LPDIRECT3DVERTEXBUFFER9 d3dVertexBuffer = nullptr;
    HRESULT hResult = d3dDevice->CreateVertexBuffer(size, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT, &d3dVertexBuffer, nullptr);
    if (hResult != S_OK)
        return 0;
    return reinterpret_cast<uint64_t>(d3dVertexBuffer) | D3DRTYPE_VERTEXBUFFER;
}
//------------------------------------------------------------------------------
void xxDestroyBufferD3D9(uint64_t buffer)
{
    switch (getResourceType(buffer))
    {
    case 0:
    {
        char* d3dBuffer = reinterpret_cast<char*>(buffer);
        xxFree(d3dBuffer);
        break;
    }
    case D3DRTYPE_VERTEXBUFFER:
    case D3DRTYPE_INDEXBUFFER:
    {
        LPDIRECT3DRESOURCE9 d3dResource = reinterpret_cast<LPDIRECT3DRESOURCE9>(getResourceData(buffer));
        if (d3dResource == nullptr)
            return;
        d3dResource->Release();
        break;
    }
    default:
        break;
    }
}
//------------------------------------------------------------------------------
void* xxMapBufferD3D9(uint64_t buffer)
{
    switch (getResourceType(buffer))
    {
    case 0:
    {
        BYTE* ptr = reinterpret_cast<BYTE*>(buffer);
        return ptr;
    }
    case D3DRTYPE_VERTEXBUFFER:
    {
        LPDIRECT3DVERTEXBUFFER9 d3dVertexBuffer = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9>(getResourceData(buffer));
        if (d3dVertexBuffer == nullptr)
            break;

        void* ptr = nullptr;
        HRESULT hResult = d3dVertexBuffer->Lock(0, 0, &ptr, D3DLOCK_NOOVERWRITE);
        if (hResult == S_OK)
            return ptr;
        break;
    }
    case D3DRTYPE_INDEXBUFFER:
    {
        LPDIRECT3DINDEXBUFFER9 d3dIndexBuffer = reinterpret_cast<LPDIRECT3DINDEXBUFFER9>(getResourceData(buffer));
        if (d3dIndexBuffer == nullptr)
            break;

        void* ptr = nullptr;
        HRESULT hResult = d3dIndexBuffer->Lock(0, 0, &ptr, D3DLOCK_NOOVERWRITE);
        if (hResult == S_OK)
            return ptr;
        break;
    }
    default:
        break;
    }

    return nullptr;
}
//------------------------------------------------------------------------------
void xxUnmapBufferD3D9(uint64_t buffer)
{
    switch (getResourceType(buffer))
    {
    case 0:
    {
        return;
    }
    case D3DRTYPE_VERTEXBUFFER:
    {
        LPDIRECT3DVERTEXBUFFER9 d3dVertexBuffer = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9>(getResourceData(buffer));
        if (d3dVertexBuffer == nullptr)
            break;

        d3dVertexBuffer->Unlock();
        return;
    }
    case D3DRTYPE_INDEXBUFFER:
    {
        LPDIRECT3DINDEXBUFFER9 d3dIndexBuffer = reinterpret_cast<LPDIRECT3DINDEXBUFFER9>(getResourceData(buffer));
        if (d3dIndexBuffer == nullptr)
            break;

        d3dIndexBuffer->Unlock();
        return;
    }
    default:
        break;
    }
}
//==============================================================================
//  Texture
//==============================================================================
uint64_t xxCreateTextureD3D9(uint64_t device, int format, unsigned int width, unsigned int height, unsigned int depth, unsigned int mipmap, unsigned int array)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(device);
    if (d3dDevice == nullptr)
        return 0;

    if (width == 0 || height == 0 || depth == 0 || mipmap == 0 || array == 0)
        return 0;

    if (depth == 1 && array == 1)
    {
        LPDIRECT3DTEXTURE9 d3dTexture = nullptr;
        HRESULT hResult = d3dDevice->CreateTexture(width, height, mipmap, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &d3dTexture, nullptr);
        if (hResult != S_OK)
            return 0;
        return reinterpret_cast<uint64_t>(d3dTexture) | D3DRTYPE_TEXTURE;
    }

    if (width == height && depth == 1 && array == 6)
    {
        LPDIRECT3DCUBETEXTURE9 d3dCubeTexture = nullptr;
        HRESULT hResult = d3dDevice->CreateCubeTexture(width, mipmap, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &d3dCubeTexture, nullptr);
        if (hResult != S_OK)
            return 0;
        return reinterpret_cast<uint64_t>(d3dCubeTexture) | D3DRTYPE_CUBETEXTURE;
    }

    if (depth != 1 && array == 1)
    {
        LPDIRECT3DVOLUMETEXTURE9 d3dVolumeTexture = nullptr;
        HRESULT hResult = d3dDevice->CreateVolumeTexture(width, height, depth, mipmap, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &d3dVolumeTexture, nullptr);
        if (hResult != S_OK)
            return 0;
        return reinterpret_cast<uint64_t>(d3dVolumeTexture) | D3DRTYPE_VOLUME;
    }

    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyTextureD3D9(uint64_t texture)
{
    LPDIRECT3DBASETEXTURE9 d3dBaseTexture = reinterpret_cast<LPDIRECT3DBASETEXTURE9>(getResourceData(texture));
    if (d3dBaseTexture == nullptr)
        return;
    d3dBaseTexture->Release();
}
//------------------------------------------------------------------------------
void* xxMapTextureD3D9(uint64_t texture, unsigned int& stride, unsigned int mipmap, unsigned int array)
{
    switch (getResourceType(texture))
    {
    case D3DRTYPE_TEXTURE:
    {
        LPDIRECT3DTEXTURE9 d3dTexture = reinterpret_cast<LPDIRECT3DTEXTURE9>(getResourceData(texture));
        if (d3dTexture == nullptr)
            break;

        D3DLOCKED_RECT rect = {};
        d3dTexture->LockRect(mipmap, &rect, nullptr, D3DLOCK_NOSYSLOCK);
        stride = rect.Pitch;
        return rect.pBits;
    }
    case D3DRTYPE_CUBETEXTURE:
    {
        LPDIRECT3DCUBETEXTURE9 d3dCubeTexture = reinterpret_cast<LPDIRECT3DCUBETEXTURE9>(getResourceData(texture));
        if (d3dCubeTexture == nullptr)
            break;

        D3DLOCKED_RECT rect = {};
        d3dCubeTexture->LockRect((D3DCUBEMAP_FACES)array, mipmap, &rect, nullptr, D3DLOCK_NOSYSLOCK);
        stride = rect.Pitch;
        return rect.pBits;
    }
    case D3DRTYPE_VOLUME:
    {
        LPDIRECT3DVOLUMETEXTURE9 d3dVolumeTexture = reinterpret_cast<LPDIRECT3DVOLUMETEXTURE9>(getResourceData(texture));
        if (d3dVolumeTexture == nullptr)
            break;

        D3DLOCKED_BOX box = {};
        d3dVolumeTexture->LockBox(mipmap, &box, nullptr, D3DLOCK_NOSYSLOCK);
        stride = box.RowPitch;
        return box.pBits;
    }
    default:
        break;
    }

    return nullptr;
}
//------------------------------------------------------------------------------
void xxUnmapTextureD3D9(uint64_t texture, unsigned int mipmap, unsigned int array)
{
    switch (getResourceType(texture))
    {
    case D3DRTYPE_TEXTURE:
    {
        LPDIRECT3DTEXTURE9 d3dTexture = reinterpret_cast<LPDIRECT3DTEXTURE9>(getResourceData(texture));
        if (d3dTexture == nullptr)
            break;

        d3dTexture->UnlockRect(mipmap);
        return;
    }
    case D3DRTYPE_CUBETEXTURE:
    {
        LPDIRECT3DCUBETEXTURE9 d3dCubeTexture = reinterpret_cast<LPDIRECT3DCUBETEXTURE9>(getResourceData(texture));
        if (d3dCubeTexture == nullptr)
            break;

        d3dCubeTexture->UnlockRect((D3DCUBEMAP_FACES)array, mipmap);
        return;
    }
    case D3DRTYPE_VOLUME:
    {
        LPDIRECT3DVOLUMETEXTURE9 d3dVolumeTexture = reinterpret_cast<LPDIRECT3DVOLUMETEXTURE9>(getResourceData(texture));
        if (d3dVolumeTexture == nullptr)
            break;

        d3dVolumeTexture->UnlockBox(mipmap);
        return;
    }
    default:
        break;
    }
}
//==============================================================================
//  Vertex Attribute
//==============================================================================
union D3DVERTEXATTRIBUTE9
{
    uint64_t value;
    struct
    {
        DWORD fvf;
        int stride;
    };
};
//------------------------------------------------------------------------------
uint64_t xxCreateVertexAttributeD3D9(uint64_t device, int count, ...)
{
    D3DVERTEXATTRIBUTE9 d3dVertexAttribtue = {};

    int stride = 0;

    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; ++i)
    {
        int stream = va_arg(args, int);
        int offset = va_arg(args, int);
        int element = va_arg(args, int);
        int size = va_arg(args, int);

        stride += size;

        if (offset == 0 && element == 3 && size == sizeof(float) * 3)
            d3dVertexAttribtue.fvf |= D3DFVF_XYZ;
        if (offset != 0 && element == 3 && size == sizeof(float) * 3)
            d3dVertexAttribtue.fvf |= D3DFVF_NORMAL;
        if (offset != 0 && element == 4 && size == sizeof(char) * 4)
            d3dVertexAttribtue.fvf |= D3DFVF_DIFFUSE;
        if (offset != 0 && element == 2 && size == sizeof(float) * 2)
            d3dVertexAttribtue.fvf += D3DFVF_TEX1;
    }
    va_end(args);

    d3dVertexAttribtue.stride = stride;

    return d3dVertexAttribtue.value;
}
//------------------------------------------------------------------------------
void xxDestroyVertexAttributeD3D9(uint64_t vertexAttribute)
{

}
//==============================================================================
//  Shader
//==============================================================================
uint64_t xxCreateVertexShaderD3D9(uint64_t device, const char* shader, uint64_t vertexAttribute)
{
    return 0;
}
//------------------------------------------------------------------------------
uint64_t xxCreateFragmentShaderD3D9(uint64_t device, const char* shader)
{
    return 0;
}
//------------------------------------------------------------------------------
void xxDestroyShaderD3D9(uint64_t device, uint64_t shader)
{

}
//==============================================================================
//  Command
//==============================================================================
void xxSetViewportD3D9(uint64_t commandBuffer, int x, int y, int width, int height, float minZ, float maxZ)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;

    D3DVIEWPORT9 vp;
    vp.X = x;
    vp.Y = y;
    vp.Width = width;
    vp.Height = height;
    vp.MinZ = minZ;
    vp.MaxZ = maxZ;
    d3dDevice->SetViewport(&vp);
}
//------------------------------------------------------------------------------
void xxSetScissorD3D9(uint64_t commandBuffer, int x, int y, int width, int height)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;

    RECT rect;
    rect.left = x;
    rect.top = y;
    rect.right = x + width;
    rect.bottom = y + height;
    d3dDevice->SetScissorRect(&rect);
}
//------------------------------------------------------------------------------
void xxSetIndexBufferD3D9(uint64_t commandBuffer, uint64_t buffer)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;
    LPDIRECT3DINDEXBUFFER9 d3dIndexBuffer = reinterpret_cast<LPDIRECT3DINDEXBUFFER9>(getResourceData(buffer));
    if (d3dIndexBuffer == nullptr)
        return;

    d3dDevice->SetIndices(d3dIndexBuffer);
}
//------------------------------------------------------------------------------
void xxSetVertexBuffersD3D9(uint64_t commandBuffer, int count, const uint64_t* buffers)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;

    for (int i = 0; i < count; ++i)
    {
        LPDIRECT3DVERTEXBUFFER9 d3dVertexBuffer = reinterpret_cast<LPDIRECT3DVERTEXBUFFER9>(getResourceData(buffers[i]));
        d3dDevice->SetStreamSource(i, d3dVertexBuffer, 0, 0);
    }
}
//------------------------------------------------------------------------------
void xxSetFragmentBuffersD3D9(uint64_t commandBuffer, int count, const uint64_t* buffers)
{

}
//------------------------------------------------------------------------------
void xxSetVertexTexturesD3D9(uint64_t commandBuffer, int count, const uint64_t* textures)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;

    for (int i = 0; i < count; ++i)
    {
        LPDIRECT3DBASETEXTURE9 d3dBaseTexture = reinterpret_cast<LPDIRECT3DBASETEXTURE9>(getResourceData(textures[i]));
        d3dDevice->SetTexture(i + D3DVERTEXTEXTURESAMPLER0, d3dBaseTexture);
    }
}
//------------------------------------------------------------------------------
void xxSetFragmentTexturesD3D9(uint64_t commandBuffer, int count, const uint64_t* textures)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;

    for (int i = 0; i < count; ++i)
    {
        LPDIRECT3DBASETEXTURE9 d3dBaseTexture = reinterpret_cast<LPDIRECT3DBASETEXTURE9>(getResourceData(textures[i]));
        d3dDevice->SetTexture(i, d3dBaseTexture);
    }
}
//------------------------------------------------------------------------------
void xxSetVertexAttributeD3D9(uint64_t commandBuffer, uint64_t vertexAttribute)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;

    D3DVERTEXATTRIBUTE9 d3dVertexAttribtue = { vertexAttribute };
    d3dDevice->SetFVF(d3dVertexAttribtue.fvf);

    for (int i = 0; i < 8; ++i)
    {
        LPDIRECT3DVERTEXBUFFER9 d3dVertexBuffer = nullptr;
        UINT offset;
        UINT stride;
        d3dDevice->GetStreamSource(i, &d3dVertexBuffer, &offset, &stride);
        if (d3dVertexBuffer == nullptr)
            break;
        d3dDevice->SetStreamSource(i, d3dVertexBuffer, 0, d3dVertexAttribtue.stride);
        d3dVertexBuffer->Release();
    }
}
//------------------------------------------------------------------------------
void xxSetVertexShaderD3D9(uint64_t commandBuffer, uint64_t shader)
{

}
//------------------------------------------------------------------------------
void xxSetFragmentShaderD3D9(uint64_t commandBuffer, uint64_t shader)
{

}
//------------------------------------------------------------------------------
void xxSetVertexConstantBufferD3D9(uint64_t commandBuffer, uint64_t buffer, unsigned int size)
{

}
//------------------------------------------------------------------------------
void xxSetFragmentConstantBufferD3D9(uint64_t commandBuffer, uint64_t buffer, unsigned int size)
{

}
//------------------------------------------------------------------------------
void xxDrawIndexedD3D9(uint64_t commandBuffer, int indexCount, int instanceCount, int firstIndex, int vertexOffset, int firstInstance)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;

    d3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vertexOffset, 0, 0, firstIndex, indexCount / 3);
}
//==============================================================================
//  Fixed-Function
//==============================================================================
void xxSetTransformD3D9(uint64_t commandBuffer, const float* world, const float* view, const float* projection)
{
    LPDIRECT3DDEVICE9 d3dDevice = reinterpret_cast<LPDIRECT3DDEVICE9>(commandBuffer);
    if (d3dDevice == nullptr)
        return;

    if (world)
        d3dDevice->SetTransform(D3DTS_WORLD, (const D3DMATRIX*)world);
    if (view)
        d3dDevice->SetTransform(D3DTS_VIEW, (const D3DMATRIX*)view);
    if (projection)
        d3dDevice->SetTransform(D3DTS_PROJECTION, (const D3DMATRIX*)projection);
}
//==============================================================================