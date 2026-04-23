#pragma once

/*
    This is so ubiquitous in DirectX code that
    I'm not putting it into a namespace.
*/
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch DirectX API errors
        throw std::exception();
    }
}