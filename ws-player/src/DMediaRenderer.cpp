#include "DMediaRenderer.h"
#include <assert.h>
#include <iostream>

DMediaRenderer::DMediaRenderer()
    : m_wnd(NULL),
    m_renderer(NULL),
    m_texture(NULL)
{
    
}

DMediaRenderer::~DMediaRenderer()
{
    UnInit();
}

int DMediaRenderer::Init(int width, int height)
{
    printf("init renderer...\n");
    m_width = width;
    m_height = height;

    // webgpu初始化
    // instance = wgpu::CreateInstance();
	// if (!instance) {
	// 	printf("webGpu create instance error\n");
	// 	return -1;
	// }

    // GetDevice();

    // sdl初始化
    SDL_Init(SDL_INIT_VIDEO);

    SDL_SetHint(SDL_HINT_EMSCRIPTEN_KEYBOARD_ELEMENT, "#canvas");

    if (SDL_CreateWindowAndRenderer(m_width, m_height, 0, &m_wnd, &m_renderer) < 0) {
        printf("create window and renderer failed!\n");
        return -1;
    }

    SDL_SetRenderDrawColor(m_renderer, 0x30, 0x30, 0x30, 255);
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);

    printf("init renderer succeed!\n");

    return 0;
}

void DMediaRenderer::UnInit()
{
    // webgpu去初始化
    // TextureDestory(textureY);
    // TextureDestory(textureU);
    // TextureDestory(textureV);
    // DeviceDestory(device);

    // sdl去初始化
    if (m_texture) {
        SDL_DestroyTexture(m_texture);
        m_texture = NULL;
    }
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = NULL;
    }
    if (m_wnd) {
        SDL_DestroyWindow(m_wnd);
        m_wnd = NULL;
    }

    SDL_Quit();
}

int DMediaRenderer::DrawAVFrame(AVFrame* frame)
{
    // WGPU渲染
    // return WGPURender(frame);

    // SDL渲染
    return SDLRender(frame);
}

void DMediaRenderer::DeviceReady(wgpu::Device device)
{
    this->device = device;

    wgpu::SurfaceDescriptorFromCanvasHTMLSelector canvasDesc;
    canvasDesc.selector = "#canvas";

    wgpu::SurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &canvasDesc;
    wgpu::Surface surface = instance.CreateSurface(&surfaceDesc);

    SetupSwapChain(surface);
}

int DMediaRenderer::SetupSwapChain(wgpu::Surface surface) {
    wgpu::SwapChainDescriptor scDesc;
    scDesc.usage = wgpu::TextureUsage::RenderAttachment;
    scDesc.format = wgpu::TextureFormat::BGRA8Unorm;
    scDesc.width = m_width;
    scDesc.height = m_height;
    scDesc.presentMode = wgpu::PresentMode::Fifo;
    swapChain = device.CreateSwapChain(surface, &scDesc);
    if (!swapChain) {
        std::cerr << "Failed to create swap chain." << std::endl;
        return -1;
    }
    return 0;
}

wgpu::Texture DMediaRenderer::CreateTexture(const wgpu::Device& device, size_t width, size_t height) {
    wgpu::TextureDescriptor textureDesc;
    textureDesc.size.width = width;
    textureDesc.size.height = height;
    textureDesc.size.depthOrArrayLayers = 1;
    textureDesc.format = wgpu::TextureFormat::R8Unorm;  // 使用 R8 格式对于 YUV420 的每个分量来说通常足够
    textureDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopyDst;
    wgpu::Texture texture = device.CreateTexture(&textureDesc);
    return texture;
}

int DMediaRenderer::CreateYUVRenderPipeline(int width, int height) {
    // 创建采样器
    wgpu::SamplerDescriptor samplerDesc = {};
    wgpu::Sampler samplerY = device.CreateSampler(&samplerDesc);
    wgpu::Sampler samplerU = device.CreateSampler(&samplerDesc);
    wgpu::Sampler samplerV = device.CreateSampler(&samplerDesc);

    // 创建离屏渲染纹理
    textureY = CreateTexture(device, width, height);
    size_t widthUV = width / 2;
    size_t heightUV = height / 2;
    textureU = CreateTexture(device, widthUV, heightUV);
    textureV = CreateTexture(device, widthUV, heightUV);

    // 获取纹理视图
    wgpu::TextureView textureViewY = textureY.CreateView();
    wgpu::TextureView textureViewU = textureU.CreateView();
    wgpu::TextureView textureViewV = textureV.CreateView();

    // 定义绑定组布局
    wgpu::BindGroupLayoutDescriptor bglDesc;
    wgpu::BindGroupLayoutEntry entries[6];
    for (int i = 0; i < 3; ++i) { // sampler
        entries[i].binding = i;
        entries[i].visibility = wgpu::ShaderStage::Fragment;
        entries[i].sampler.type = wgpu::SamplerBindingType::Filtering;
    }
    for (int i = 3; i < 6; ++i) { // Texture
        entries[i].binding = i;
        entries[i].visibility = wgpu::ShaderStage::Fragment;
        entries[i].texture.sampleType = wgpu::TextureSampleType::Float;
    }
    bglDesc.entryCount = sizeof(entries) / sizeof(entries[0]);
    bglDesc.entries = entries;
    auto bgl = device.CreateBindGroupLayout(&bglDesc);

    // 创建绑定组
    wgpu::BindGroupDescriptor bgDesc = {};
    wgpu::BindGroupEntry bgEntries[6];
    bgEntries[0].binding = 0;
    bgEntries[0].sampler = samplerY;
    bgEntries[1].binding = 1;
    bgEntries[1].sampler = samplerU;
    bgEntries[2].binding = 2;
    bgEntries[2].sampler = samplerV;
    bgEntries[3].binding = 3;
    bgEntries[3].textureView = textureViewY;
    bgEntries[4].binding = 4;
    bgEntries[4].textureView = textureViewU;
    bgEntries[5].binding = 5;
    bgEntries[5].textureView = textureViewV;

    bgDesc.layout = bgl;
    bgDesc.entryCount = sizeof(bgEntries) / sizeof(bgEntries[0]);
    bgDesc.entries = bgEntries;
    bindGroup = device.CreateBindGroup(&bgDesc);

    // 创建着色器
    wgpu::ShaderModuleWGSLDescriptor wgslDesc;
    wgslDesc.code = YUVshaderCode;
    wgpu::ShaderModuleDescriptor shaderModuleDescriptor;
    shaderModuleDescriptor.nextInChain = &wgslDesc;
    wgpu::ShaderModule shaderModule = device.CreateShaderModule(&shaderModuleDescriptor);

    // 创建管道样式
    wgpu::PipelineLayoutDescriptor pipelineLayoutDescriptor;
    pipelineLayoutDescriptor.bindGroupLayouts = &bgl;
    pipelineLayoutDescriptor.bindGroupLayoutCount = 1;
    wgpu::PipelineLayout pipelineLayout = device.CreatePipelineLayout(&pipelineLayoutDescriptor);

    wgpu::ColorTargetState colorTargetState;
    colorTargetState.format = wgpu::TextureFormat::BGRA8Unorm;
    // colorTargetState.blend = &blendState;
    // colorTargetState.writeMask = wgpu::ColorWriteMask::All;

    wgpu::FragmentState fragmentState;
    fragmentState.module = shaderModule;
    fragmentState.entryPoint = "fs_main";// 片段着色器函数入口
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;

    wgpu::RenderPipelineDescriptor descriptor;
    // 画三角形不需要位置缓冲区
    descriptor.vertex.bufferCount = 0;
    descriptor.vertex.buffers = nullptr;

    descriptor.layout = pipelineLayout;
    descriptor.vertex.module = shaderModule;
    descriptor.vertex.entryPoint = "vs_main"; // 顶点着色器函数入口
    // 着色器常量的赋值数组暂时不使用
    descriptor.vertex.constantCount = 0;
    descriptor.vertex.constants = nullptr;

    descriptor.fragment = &fragmentState;
    pipeline = device.CreateRenderPipeline(&descriptor);
    if (!pipeline) {
        std::cerr << "CreateRenderPipeline error" << std::endl;
        return -1;
    }
    std::cout << "create render pipe success" << std::endl;
    return 0;
}

void DMediaRenderer::GetDevice() {
    instance.RequestAdapter(
        nullptr,
        AdapterCallback,
        this
    );
}

void DMediaRenderer::UploadTexture(const wgpu::Device& device, const wgpu::Texture& texture, const uint8_t* data, size_t width, size_t height, size_t bytesPerRow) {
    // 申请的缓冲区行字节必须256字节对齐
    wgpu::BufferDescriptor bufferDesc;
    bufferDesc.size = bytesPerRow * height;
    bufferDesc.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
    
    wgpu::Queue queue = device.GetQueue();
    queue.WriteBuffer(buffer, 0, data, bufferDesc.size);
    
    wgpu::ImageCopyBuffer imageCopyBuffer;
    imageCopyBuffer.buffer = buffer;
    imageCopyBuffer.layout.bytesPerRow = bytesPerRow;

    wgpu::ImageCopyTexture imageCopyTexture;
    imageCopyTexture.texture = texture;

    wgpu::Extent3D extent;
    extent.width = width;
    extent.height = height;
    extent.depthOrArrayLayers = 1;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &extent);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

void DMediaRenderer::PaddingYUV(const AVFrame* frame, 
                uint8_t** paddedY, size_t *paddedSizeY, 
                uint8_t** paddedU, size_t *paddedSizeU, 
                uint8_t** paddedV, size_t *paddedSizeV)
{
    int width = frame->width;   // 视频宽度
    int height = frame->height; // 视频高度

    // Y分量
    uint8_t* yPlane = frame->data[0];

    // U分量
    uint8_t* uPlane = frame->data[1];

    // V分量
    uint8_t* vPlane = frame->data[2];

    // 计算需要对齐的行宽
    size_t bytesPerRow = (width + 255) & ~255; // 对齐到256字节
    size_t paddedSize = bytesPerRow * height; // 计算填充后的大小

    // 分配新的数组用于存放填充后的数据
    uint8_t* paddedDataY = new uint8_t[paddedSize];
    // 初始化所有像素的Y分量
    memset(paddedDataY, 0, paddedSize);

    // 将原始数据复制到填充后的数据数组，同时在每行末尾添加填充
    for (int y = 0; y < height; ++y) {
        memcpy(paddedDataY + y * bytesPerRow, yPlane + y * width, width);
        // 剩余的部分(bytesPerRow - width)会保持初始化时的0填充
    }

    *paddedY = paddedDataY;
    *paddedSizeY = paddedSize;

    // 假设width和height是Y分量的宽度和高度
    size_t widthUV = width / 2;
    size_t heightUV = height / 2;
    size_t actualBytesPerRowUV = widthUV; // 如果你的UV数据是单字节格式（如YUV420）

    // 计算需要填充的字节以达到256字节对齐
    size_t bytesPerRowUV = (actualBytesPerRowUV + 255) & ~255;
    size_t paddedSizeUV = bytesPerRowUV * heightUV;

    // 分配内存以存储填充后的UV分量数据
    uint8_t* paddedDataU = new uint8_t[paddedSizeUV];
    uint8_t* paddedDataV = new uint8_t[paddedSizeUV];

    // 对U和V分量执行填充操作
    for (size_t row = 0; row < heightUV; ++row) {
        // 复制原始数据
        memcpy(paddedDataU + row * bytesPerRowUV, uPlane + row * actualBytesPerRowUV, actualBytesPerRowUV);
        memcpy(paddedDataV + row * bytesPerRowUV, vPlane + row * actualBytesPerRowUV, actualBytesPerRowUV);
        
        // 添加填充（无色值128）
        memset(paddedDataU + row * bytesPerRowUV + actualBytesPerRowUV, 128, bytesPerRowUV - actualBytesPerRowUV);
        memset(paddedDataV + row * bytesPerRowUV + actualBytesPerRowUV, 128, bytesPerRowUV - actualBytesPerRowUV);
    }

    *paddedU = paddedDataU;
    *paddedSizeU = paddedSizeUV;
    *paddedV = paddedDataV;
    *paddedSizeV = paddedSizeUV;
}

int DMediaRenderer::WGPURender(const AVFrame *frame) {
    if (!pipeline) {
        int ret = CreateYUVRenderPipeline(frame->width, frame->height);
        if (ret < 0)
            return -1;
        m_textureWidth = frame->width;
        m_textureHeight = frame->height;
        std::cout << "render width is : " << m_textureWidth << " render height is : " << m_textureHeight << std::endl;
    }

    wgpu::TextureView nextTexture = swapChain.GetCurrentTextureView();
    if (!nextTexture) {
        std::cerr << "Cannot acquire next swap chain texture" << std::endl;
        return -1;
    }

    uint8_t* paddingY = nullptr;
    uint8_t* paddingU = nullptr;
    uint8_t* paddingV = nullptr;
    size_t paddingSizeY = 0;
    size_t paddingSizeU = 0;
    size_t paddingSizeV = 0;

    PaddingYUV(frame, &paddingY, &paddingSizeY, &paddingU, &paddingSizeU, &paddingV, &paddingSizeV);

    // Y
    size_t bytesPerRow = (m_textureWidth + 255) & ~255; // 对齐到256字节
    UploadTexture(device, textureY, paddingY, m_textureWidth, m_textureHeight, bytesPerRow);

    // UV
    size_t widthUV = m_textureWidth / 2;
    size_t heightUV = m_textureHeight / 2;
    size_t bytesPerRowUV = (widthUV + 255) & ~255; // 对齐到256字节
    UploadTexture(device, textureU, paddingU, widthUV, heightUV, bytesPerRowUV);
    UploadTexture(device, textureV, paddingV, widthUV, heightUV, bytesPerRowUV);

    wgpu::RenderPassColorAttachment attachment;
    attachment.view = nextTexture;
    attachment.loadOp = wgpu::LoadOp::Clear;
    attachment.storeOp = wgpu::StoreOp::Store;

    attachment.clearValue = wgpu::Color{ 0.9, 0.1, 0.2, 1.0 };
    wgpu::RenderPassDescriptor renderpass;
    renderpass.colorAttachments = &attachment;
    renderpass.colorAttachmentCount = 1;

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderpass);

    pass.SetPipeline(pipeline);
    // pass.Draw(3);
    pass.SetBindGroup(0, bindGroup);
    pass.Draw(6);
    pass.End();

    wgpu::CommandBuffer commands = encoder.Finish();
    device.GetQueue().Submit(1, &commands);

    delete[] paddingY;
    delete[] paddingU;
    delete[] paddingV;
    return 0;
}

int DMediaRenderer::SDLRender(const AVFrame *frame)
{
    assert(frame != NULL);

    if (frame->width != m_textureWidth || frame->height != m_textureHeight) {
        if (m_texture) {
            SDL_DestroyTexture(m_texture);
            m_texture = NULL;
        }
    }

    if (!m_texture) {
        if (!m_renderer) {
            printf("renderer not initialized!\n");
            return -1;
        }
        printf("try create texture %d*%d ...\n", frame->width, frame->height);
        m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, frame->width, frame->height);
        if (m_texture) {
            printf("create texture succeed!\n");
            m_textureWidth = frame->width;
            m_textureHeight = frame->height;
        }
        else {
            printf("create texture failed!\n");
        }
    }
    if (!m_texture) {
        printf("texture not initialized!\n");
        return -1;
    }

    int ret = SDL_UpdateYUVTexture(m_texture, NULL, frame->data[0], frame->linesize[0],
        frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);
    if (ret != 0) {
        printf("SDL_UpdateYUVTexture error!message is : %s\n", SDL_GetError());
        return -1;
    }
    ret = SDL_RenderClear(m_renderer);
    if (ret != 0) {
        printf("SDL_RenderClear error!message is : %s\n", SDL_GetError());
        return -1;
    }
    ret = SDL_RenderCopy(m_renderer, m_texture, NULL, NULL);
    if (ret != 0) {
        printf("YUV SDL_RenderCopy error!message is : %s\n", SDL_GetError());
        return -1;
    }

    SDL_RenderPresent(m_renderer);
    // printf("SDL_RenderPresent success!\n");
    return 0;
}

void DMediaRenderer::TextureDestory(wgpu::Texture& texture) {
    if (texture)
        texture.Destroy();
}

void DMediaRenderer::DeviceDestory(wgpu::Device& device) {
    if (device)
        device.Destroy();
}