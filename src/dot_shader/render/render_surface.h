#pragma once
#include "directx/d3d12sdklayers.h"
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <windows.h>

namespace DotShader::Render {

struct Vertex {
    float position[3];
    float color[4];
};

class RenderSurface {
    template<class T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

  public:
    RenderSurface(HWND hwnd);

    void render_frame();

  private:
    void load_pipeline(HWND hwnd);
    void load_assets();
    void wait_for_previous_frame();
    void populate_command_list();

    static constexpr UINT FrameCount{ 2 };

    UINT m_width { 1000 };
    UINT m_height{ 1000 };

    ComPtr<ID3D12Device>              m_device;
    ComPtr<ID3D12CommandQueue>        m_command_queue;
    ComPtr<IDXGISwapChain3>           m_swap_chain;
    ComPtr<ID3D12Resource>            m_render_targets[FrameCount];
    ComPtr<ID3D12CommandAllocator>    m_command_allocator;
    ComPtr<ID3D12DescriptorHeap>      m_rtv_heap;
    UINT                              m_rtv_descriptor_size{};
    ComPtr<ID3D12GraphicsCommandList> m_command_list;

    D3D12_VIEWPORT m_viewport;
    RECT           m_scissor_rect;

    ComPtr<ID3D12RootSignature> m_root_signature;
    ComPtr<ID3D12PipelineState> m_pipeline_state;

    ComPtr<ID3D12Resource>   m_vertex_buffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertex_buffer_view;

    UINT                m_frame_index{};
    HANDLE              m_fence_event;
    ComPtr<ID3D12Fence> m_fence;
    UINT64              m_fence_value;
};

}