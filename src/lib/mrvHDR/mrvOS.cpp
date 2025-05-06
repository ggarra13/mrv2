
#ifdef _WIN32
#    include <winsock2.h>
#    include <windows.h>
#    include <dxgi1_6.h>
#endif

#include <iostream>

namespace mrv
{

#ifdef _WIN32
    bool is_hdr_display_active()
    {
        IDXGIFactory6* factory = nullptr;
        CreateDXGIFactory1(IID_PPV_ARGS(&factory));

        IDXGIAdapter1* adapter = nullptr;
        for (UINT i = 0;
             factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            IDXGIOutput* output = nullptr;
            if (SUCCEEDED(adapter->EnumOutputs(0, &output)))
            {
                IDXGIOutput6* output6 = nullptr;
                if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(&output6))))
                {
                    DXGI_OUTPUT_DESC1 desc;
                    output6->GetDesc1(&desc);

                    // Check for HDR support
                    if (desc.ColorSpace ==
                            DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020 ||
                        desc.ColorSpace ==
                            DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709)
                    {
                        std::cout << "HDR is active on display.\n";
                        output6->Release();
                        output->Release();
                        adapter->Release();
                        factory->Release();
                        return true;
                    }

                    output6->Release();
                }
                output->Release();
            }
            adapter->Release();
        }
        factory->Release();
        return false;
    }

#endif

#ifdef __linux__
    bool is_hdr_display_active()
    {
        return true;
    }
#endif

} // namespace mrv
