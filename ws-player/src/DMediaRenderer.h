#pragma once

#include "SDL2/SDL.h"
#include "inc-ffmpeg.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include <webgpu/webgpu_cpp.h>

using namespace std;

class DMediaRenderer
{
public:
    DMediaRenderer();
    virtual ~DMediaRenderer();

public:
    int Init(int width, int height);
    void UnInit();
    int DrawAVFrame(AVFrame *frame);

protected:
    void UploadTexture(const wgpu::Device& device, const wgpu::Texture& texture, 
                        const uint8_t* data, size_t width, size_t height, size_t bytesPerRow);

    void GetDevice();

    void DeviceReady(wgpu::Device device);

    int SetupSwapChain(wgpu::Surface surface);

    int CreateYUVRenderPipeline(int width, int height);

    wgpu::Texture CreateTexture(const wgpu::Device& device, size_t width, size_t height);

    void PaddingYUV(const AVFrame* frame, 
                    uint8_t** paddedY, size_t *paddedSizeY, 
                    uint8_t** paddedU, size_t *paddedSizeU, 
                    uint8_t** paddedV, size_t *paddedSizeV);

    int WGPURender(const AVFrame *frame);

    int SDLRender(const AVFrame *frame);

    void TextureDestory(wgpu::Texture& texture);

    void DeviceDestory(wgpu::Device& device);

    static void AdapterCallback(WGPURequestAdapterStatus status, WGPUAdapter cAdapter, const char *message, void *userdata) {
        auto renderer = reinterpret_cast<DMediaRenderer*>(userdata);
        if (status != WGPURequestAdapterStatus_Success) {
            std::cout << "Adapter request status: " << status << " - error occurred" << std::endl;
            return;
        }
        wgpu::Adapter adapter = wgpu::Adapter::Acquire(cAdapter);
        adapter.RequestDevice(
            nullptr,
            DeviceCallback,
            userdata
        );
    }

    static void DeviceCallback(WGPURequestDeviceStatus status, WGPUDevice cDevice, const char *message, void *userdata) {
        auto renderer = reinterpret_cast<DMediaRenderer*>(userdata);
        if (status != WGPURequestDeviceStatus_Success) {
            std::cout << "Device request status: " << status << " - error occurred" << std::endl;
            return;
        }
        wgpu::Device device = wgpu::Device::Acquire(cDevice);
        device.SetUncapturedErrorCallback(ErrorCallback, nullptr);
        renderer->DeviceReady(device);
    }

    static void ErrorCallback(WGPUErrorType type, const char *message, void *userdata) {
        std::cout << "Error: " << type << " - " << message << std::endl;
    }

private:
    int m_width = 0;    //renderer width.
    int m_height = 0;   //renderer height.
    int m_textureWidth = 0; // 纹理宽度
    int m_textureHeight = 0; // 纹理高度

    // sdl
    SDL_Window *m_wnd;
    SDL_Renderer *m_renderer;
    SDL_Texture *m_texture;

    // webgpu
    wgpu::Instance instance;
    wgpu::Device device;
    wgpu::SwapChain swapChain;
    wgpu::RenderPipeline pipeline;
    wgpu::BindGroup bindGroup;
    wgpu::Texture textureY;
    wgpu::Texture textureU;
    wgpu::Texture textureV;
    const char* YUVshaderCode = R"(
        @group(0) @binding(0) var ySampler: sampler;
        @group(0) @binding(1) var uSampler: sampler;
        @group(0) @binding(2) var vSampler: sampler;
        @group(0) @binding(3) var yTexture: texture_2d<f32>;
        @group(0) @binding(4) var uTexture: texture_2d<f32>;
        @group(0) @binding(5) var vTexture: texture_2d<f32>;

        struct VertexOutput {
            @builtin(position) position : vec4<f32>,
            @location(0) texCoords : vec2<f32>,
            @location(1) uvTexCoords: vec2<f32>
        };

        @vertex
        fn vs_main(@builtin(vertex_index) vertexIndex: u32) -> VertexOutput {
            var output : VertexOutput;
            var positions = array<vec4<f32>, 6>(
                vec4<f32>(-1.0, 1.0, 0.0, 1.0),  // Bottom left
                vec4<f32>(1.0, 1.0, 0.0, 1.0),   // Bottom right
                vec4<f32>(-1.0, -1.0, 0.0, 1.0), // Top left
                vec4<f32>(1.0, 1.0, 0.0, 1.0),   // Bottom right
                vec4<f32>(-1.0, -1.0, 0.0, 1.0), // Top left
                vec4<f32>(1.0, -1.0, 0.0, 1.0)   // Top right
            );

            var texCoords = array<vec2<f32>, 6>(
                vec2<f32>(0.0, 0.0),  // Bottom left
                vec2<f32>(1.0, 0.0),  // Bottom right
                vec2<f32>(0.0, 1.0),  // Top left
                vec2<f32>(1.0, 0.0),  // Bottom right
                vec2<f32>(0.0, 1.0),  // Top left
                vec2<f32>(1.0, 1.0)   // Top right
            );

            // 调整UV纹理坐标
            var uvTexCoords = array<vec2<f32>, 6>(
                vec2<f32>(0.0, 0.5),
                vec2<f32>(0.5, 0.5),
                vec2<f32>(0.0, 0.0),
                vec2<f32>(0.5, 0.5),
                vec2<f32>(0.0, 0.0),
                vec2<f32>(0.5, 0.0)
            );

            output.position = positions[vertexIndex];
            output.texCoords = texCoords[vertexIndex];
            output.uvTexCoords = uvTexCoords[vertexIndex];
            return output;
        }

        @fragment
        fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
            let y = textureSample(yTexture, ySampler, in.texCoords).r;
            let u = textureSample(uTexture, uSampler, in.texCoords).r - 0.5;
            let v = textureSample(vTexture, vSampler, in.texCoords).r - 0.5;

            let r = y + 1.402 * v;
            let g = y - 0.344 * u - 0.714 * v;
            let b = y + 1.772 * u;

            return vec4<f32>(r, g, b, 1.0);
        }
    )";
};