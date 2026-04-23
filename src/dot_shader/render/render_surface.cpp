#include "render_surface.h"
#include <dxgi.h>
#include <D3DCompiler.h>
#include "directx/d3d12.h"
#include "directx/d3dx12.h"
#include "dot_shader/fs/fs.h"
#include "dot_shader/helpers/ascii_to_w.h"
#include "dot_shader/helpers/throw_if_failed.h"

namespace DotShader::Render {

RenderSurface::RenderSurface(HWND hwnd)
    : m_viewport({
        .TopLeftX = 0.0f,
        .TopLeftY = 0.0f,
        .Width    = 1000.0f,
        .Height   = 1000.0f,
        .MinDepth = 0.0f,
        .MaxDepth = 1.0f
    })
    , m_scissor_rect({
        .left   = 0,
        .top    = 0,
        .right  = 1000,
        .bottom = 1000
    }) {
    load_pipeline(hwnd);
    load_assets();
    render_frame();
}

void RenderSurface::render_frame() {
    populate_command_list();

    // Execute the command list.
    ID3D12CommandList* command_list = m_command_list.Get();
    m_command_queue->ExecuteCommandLists(
        1, &command_list);

    // Present the frame.
    ThrowIfFailed(m_swap_chain->Present(1, 0));
    wait_for_previous_frame();
}

void RenderSurface::load_pipeline(HWND hwnd) {
#ifndef NDEBUG
    std::cout << "Enabling debug layer..." << std::endl;

    ComPtr<ID3D12Debug> debug_controller;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
    debug_controller->EnableDebugLayer();
#endif

    ComPtr<IDXGIFactory4> factory;
    ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

    ComPtr<IDXGIAdapter> warp_adapter;
    ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp_adapter)));

    ThrowIfFailed(D3D12CreateDevice(
        warp_adapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)));

    D3D12_COMMAND_QUEUE_DESC queue_desc{
        .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
        .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE
    };

    ThrowIfFailed(m_device->CreateCommandQueue(
        &queue_desc,
        IID_PPV_ARGS(&m_command_queue)));

    DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
    swap_chain_desc.BufferCount       = FrameCount;
    swap_chain_desc.BufferDesc.Width  = m_width;
    swap_chain_desc.BufferDesc.Height = m_height;
    swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_desc.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_desc.SwapEffect        = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swap_chain_desc.OutputWindow      = hwnd;
    swap_chain_desc.SampleDesc.Count  = 1;
    swap_chain_desc.Windowed          = TRUE;

    ComPtr<IDXGISwapChain> swap_chain;
    ThrowIfFailed(factory->CreateSwapChain(
        m_command_queue.Get(),
        &swap_chain_desc,
        &swap_chain));

    ThrowIfFailed(swap_chain.As(&m_swap_chain));
    ThrowIfFailed(factory->MakeWindowAssociation(
        hwnd,
        DXGI_MWA_NO_ALT_ENTER));
    m_frame_index = m_swap_chain->GetCurrentBackBufferIndex();

    // Create descriptor heaps.
    {
        // Describe and create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
        rtv_heap_desc.NumDescriptors = FrameCount;
        rtv_heap_desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_heap_desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&m_rtv_heap)));

        m_rtv_descriptor_size = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_heap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < FrameCount; n++)
        {
            ThrowIfFailed(m_swap_chain->GetBuffer(n, IID_PPV_ARGS(&m_render_targets[n])));
            m_device->CreateRenderTargetView(m_render_targets[n].Get(), nullptr, rtv_handle);
            rtv_handle.Offset(1, m_rtv_descriptor_size);
        }
    }

    ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_command_allocator)));
}

void RenderSurface::load_assets() {
    // Create an empty root signature.
    {
        CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc;
        root_signature_desc.Init(
            0,
            nullptr,
            0,
            nullptr,
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_root_signature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertex_shader;
        ComPtr<ID3DBlob> pixel_shader;

#ifndef NDEBUG
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compileFlags = 0;
#endif

        ThrowIfFailed(D3DCompileFromFile(Helpers::ascii_to_w(FS::FS::resource_full_path("shaders.hlsl")).c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertex_shader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(Helpers::ascii_to_w(FS::FS::resource_full_path("shaders.hlsl")).c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixel_shader,  nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
        pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
        pso_desc.pRootSignature = m_root_signature.Get();
        pso_desc.VS = { reinterpret_cast<UINT8*>(vertex_shader->GetBufferPointer()), vertex_shader->GetBufferSize() };
        pso_desc.PS = { reinterpret_cast<UINT8*>(pixel_shader->GetBufferPointer()),  pixel_shader->GetBufferSize() };
        pso_desc.RasterizerState                 = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        pso_desc.BlendState                      = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        pso_desc.DepthStencilState.DepthEnable   = FALSE;
        pso_desc.DepthStencilState.StencilEnable = FALSE;
        pso_desc.SampleMask                      = UINT_MAX;
        pso_desc.PrimitiveTopologyType           = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso_desc.NumRenderTargets                = 1;
        pso_desc.RTVFormats[0]                   = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso_desc.SampleDesc.Count                = 1;
        ThrowIfFailed(m_device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_pipeline_state)));

        // Create the command list.
        ThrowIfFailed(m_device->CreateCommandList(
            0,
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            m_command_allocator.Get(),
            m_pipeline_state.Get(),
            IID_PPV_ARGS(&m_command_list)));

        // Command lists are created in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        ThrowIfFailed(m_command_list->Close());

        // Create the vertex buffer.
        {
            // Define the geometry for a triangle.
            Vertex triangle_vertices[2][3] = {
                {
                    { { -0.50f, -0.50f, 0.00f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
                    { { -0.50f,  0.50f, 0.00f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                    { {  0.50f,  0.50f, 0.00f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
                },
                {
                    { { -0.50f, -0.50f, 0.00f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
                    { {  0.50f,  0.50f, 0.00f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                    { {  0.50f, -0.50f, 0.00f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
                }
            };

            const UINT vertex_buffer_size = sizeof(triangle_vertices);

            // Note: using upload heaps to transfer static data like vert buffers is not
            // recommended. Every time the GPU needs it, the upload heap will be marshalled
            // over. Please read up on Default Heap usage. An upload heap is used here for
            // code simplicity and because there are very few verts to actually transfer.
            CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
            auto desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size);
            ThrowIfFailed(m_device->CreateCommittedResource(
                &heap_props,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_vertex_buffer)));

            // Copy the triangle data to the vertex buffer.
            UINT8* vertex_data_begin;
            CD3DX12_RANGE read_range(0, 0); // We do not intend to read from this resource on the CPU.
            ThrowIfFailed(m_vertex_buffer->Map(0, &read_range, reinterpret_cast<void**>(&vertex_data_begin)));
            memcpy(vertex_data_begin, triangle_vertices, sizeof(triangle_vertices));
            m_vertex_buffer->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            m_vertex_buffer_view.BufferLocation = m_vertex_buffer->GetGPUVirtualAddress();
            m_vertex_buffer_view.StrideInBytes  = sizeof(Vertex);
            m_vertex_buffer_view.SizeInBytes    = vertex_buffer_size;
        }

        // Create synchronization objects and wait until assets have been uploaded to the GPU.
        {
            ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
            m_fence_value = 1;

            // Create an event handle to use for frame synchronization.
            m_fence_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_fence_event == nullptr)
                ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));

            // Wait for the command list to execute; we are reusing the same command
            // list in our main loop but for now, we just want to wait for setup to
            // complete before continuing.
            wait_for_previous_frame();
        }
    }
}

void RenderSurface::wait_for_previous_frame() {
    //// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    //// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
    //// sample illustrates how to use fences for efficient resource usage and to
    //// maximize GPU utilization.

    //// Signal and increment the fence value.
    const UINT64 fence = m_fence_value;
    ThrowIfFailed(m_command_queue->Signal(m_fence.Get(), fence));
    m_fence_value++;

    //// Wait until the previous frame is finished.
    if (m_fence->GetCompletedValue() < fence)
    {
       ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fence_event));
       WaitForSingleObject(m_fence_event, INFINITE);
    }

    m_frame_index = m_swap_chain->GetCurrentBackBufferIndex();
}

void RenderSurface::populate_command_list()
{
    // Command list allocators can only be reset when the associated
    // command lists have finished execution on the GPU; apps should use
    // fences to determine GPU execution progress.
    ThrowIfFailed(m_command_allocator->Reset());

    // However, when ExecuteCommandList() is called on a particular command
    // list, that command list can then be reset at any time and must be before
    // re-recording.
    ThrowIfFailed(m_command_list->Reset(m_command_allocator.Get(), m_pipeline_state.Get()));

    // Set necessary state.
    m_command_list->SetGraphicsRootSignature(m_root_signature.Get());
    m_command_list->RSSetViewports(1, &m_viewport);
    m_command_list->RSSetScissorRects(1, &m_scissor_rect);

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_render_targets[m_frame_index].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_command_list->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtv_heap->GetCPUDescriptorHandleForHeapStart(), m_frame_index, m_rtv_descriptor_size);
    m_command_list->OMSetRenderTargets(1, &rtv_handle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_command_list->ClearRenderTargetView(rtv_handle, clearColor, 0, nullptr);
    m_command_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_command_list->IASetVertexBuffers(0, 1, &m_vertex_buffer_view);
    m_command_list->DrawInstanced(6, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_render_targets[m_frame_index].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);
    m_command_list->ResourceBarrier(1, &barrier);

    ThrowIfFailed(m_command_list->Close());
}

}