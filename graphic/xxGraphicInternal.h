#pragma once

#define xxRegisterFunction(API) \
    xxCreateInstance = xxCreateInstance ## API; \
    xxDestroyInstance = xxDestroyInstance ## API; \
\
    xxCreateDevice = xxCreateDevice ## API; \
    xxDestroyDevice = xxDestroyDevice ## API; \
    xxResetDevice = xxResetDevice ## API; \
    xxTestDevice = xxTestDevice ## API; \
    xxGetDeviceString = xxGetDeviceString ## API; \
\
    xxCreateSwapchain = xxCreateSwapchain ## API; \
    xxDestroySwapchain = xxDestroySwapchain ## API; \
    xxPresentSwapchain = xxPresentSwapchain ## API; \
\
    xxGetCommandBuffer = xxGetCommandBuffer ## API; \
    xxBeginCommandBuffer = xxBeginCommandBuffer ## API; \
    xxEndCommandBuffer = xxEndCommandBuffer ## API; \
    xxSubmitCommandBuffer = xxSubmitCommandBuffer ## API; \
\
    xxCreateRenderPass = xxCreateRenderPass ## API; \
    xxDestroyRenderPass = xxDestroyRenderPass ## API; \
    xxBeginRenderPass = xxBeginRenderPass ## API; \
    xxEndRenderPass = xxEndRenderPass ## API; \
\
    xxCreateConstantBuffer = xxCreateConstantBuffer ## API; \
    xxCreateIndexBuffer = xxCreateIndexBuffer ## API; \
    xxCreateVertexBuffer = xxCreateVertexBuffer ## API; \
    xxDestroyBuffer = xxDestroyBuffer ## API; \
    xxMapBuffer = xxMapBuffer ## API; \
    xxUnmapBuffer = xxUnmapBuffer ## API; \
\
    xxCreateTexture = xxCreateTexture ## API; \
    xxDestroyTexture = xxDestroyTexture ## API; \
    xxMapTexture = xxMapTexture ## API; \
    xxUnmapTexture = xxUnmapTexture ## API; \
\
    xxCreateVertexAttribute = xxCreateVertexAttribute ## API; \
    xxDestroyVertexAttribute = xxDestroyVertexAttribute ## API; \
\
    xxCreateVertexShader = xxCreateVertexShader ## API; \
    xxCreateFragmentShader = xxCreateFragmentShader ## API; \
    xxDestroyShader = xxDestroyShader ## API; \
\
    xxSetViewport = xxSetViewport ## API; \
    xxSetScissor = xxSetScissor ## API; \
    xxSetIndexBuffer = xxSetIndexBuffer ## API; \
    xxSetVertexBuffers = xxSetVertexBuffers ## API; \
    xxSetFragmentBuffers = xxSetFragmentBuffers ## API; \
    xxSetVertexTextures = xxSetVertexTextures ## API; \
    xxSetFragmentTextures = xxSetFragmentTextures ## API; \
    xxSetVertexAttribute = xxSetVertexAttribute ## API; \
    xxSetVertexShader = xxSetVertexShader ## API; \
    xxSetFragmentShader = xxSetFragmentShader ## API; \
    xxSetVertexConstantBuffer = xxSetVertexConstantBuffer ## API; \
    xxSetFragmentConstantBuffer = xxSetFragmentConstantBuffer ## API; \
    xxDrawIndexed = xxDrawIndexed ## API; \
\
    xxSetTransform = xxSetTransform ## API;

#define xxUnregisterFunction() \
    xxCreateInstance = nullptr; \
    xxDestroyInstance = nullptr; \
\
    xxCreateDevice = nullptr; \
    xxDestroyDevice = nullptr; \
    xxResetDevice = nullptr; \
    xxTestDevice = nullptr; \
    xxGetDeviceString = nullptr; \
\
    xxCreateSwapchain = nullptr; \
    xxDestroySwapchain = nullptr; \
    xxPresentSwapchain = nullptr; \
\
    xxGetCommandBuffer = nullptr; \
    xxBeginCommandBuffer = nullptr; \
    xxEndCommandBuffer = nullptr; \
    xxSubmitCommandBuffer = nullptr; \
\
    xxCreateRenderPass = nullptr; \
    xxDestroyRenderPass = nullptr; \
    xxBeginRenderPass = nullptr; \
    xxEndRenderPass = nullptr; \
\
    xxCreateConstantBuffer = nullptr; \
    xxCreateIndexBuffer = nullptr; \
    xxCreateVertexBuffer = nullptr; \
    xxDestroyBuffer = nullptr; \
    xxMapBuffer = nullptr; \
    xxUnmapBuffer = nullptr; \
\
    xxCreateTexture = nullptr; \
    xxDestroyTexture = nullptr; \
    xxMapTexture = nullptr; \
    xxUnmapTexture = nullptr; \
\
    xxCreateVertexAttribute = nullptr; \
    xxDestroyVertexAttribute = nullptr; \
\
    xxCreateVertexShader = nullptr; \
    xxCreateFragmentShader = nullptr; \
    xxDestroyShader = nullptr; \
\
    xxSetViewport = nullptr; \
    xxSetScissor = nullptr; \
    xxSetIndexBuffer = nullptr; \
    xxSetVertexBuffers = nullptr; \
    xxSetFragmentBuffers = nullptr; \
    xxSetVertexTextures = nullptr; \
    xxSetFragmentTextures = nullptr; \
    xxSetVertexAttribute = nullptr; \
    xxSetVertexShader = nullptr; \
    xxSetFragmentShader = nullptr; \
    xxSetVertexConstantBuffer = nullptr; \
    xxSetFragmentConstantBuffer = nullptr; \
    xxDrawIndexed = nullptr; \
\
    xxSetTransform = nullptr;