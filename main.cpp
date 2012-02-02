#include <windows.h>
#include <assert.h>
#include <memory>
#include <string>


class WindowsAPI
{
public:
    template<class WINDOW>
    static HWND create(WINDOW *window, const std::string &className, const std::string &title)
    {
        return CreateWindowEx(0,
                className.c_str(),
                title.c_str(),
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                NULL,
                NULL,
                GetModuleHandle(NULL),
                window);
    }

    template<class WINDOW>
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch(message)
        {
            case WM_CREATE:
                {
                    WINDOW *window=(WINDOW*)((LPCREATESTRUCT)lParam)->lpCreateParams;
                    SetWindowLongPtr(hwnd, GWL_USERDATA, (LONG_PTR)window);
                    SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)WINDOW::WndProc);
                    return window->InstanceProc(hwnd, message, wParam, lParam);
                }

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    static int loop()
    {
        MSG msg;
        while(GetMessage(&msg, NULL, 0, 0) != 0){
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
        return msg.wParam;
    }
};


template<class WINDOW>
bool register_class(const std::string &className)
{
    WNDCLASSEX wndclass;
    ZeroMemory(&wndclass, sizeof(wndclass));
    wndclass.cbSize=sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WindowsAPI::WndProc<WINDOW>;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = GetModuleHandle(NULL);
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = className.c_str();
    return RegisterClassEx(&wndclass)!=0;
}


class Window
{
    HWND hwnd_;
public:
    Window()
        : hwnd_(0)
    {
    }

    void setHwnd(HWND hwnd)
    {
        hwnd_=hwnd;
    }

    void show()
    {
        ShowWindow(hwnd_, SW_SHOWNORMAL);
        UpdateWindow(hwnd_);
    }

    LRESULT InstanceProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch(message)
        {
            case WM_PAINT:
                {
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hwnd, &ps);
                    RECT rect;
                    GetClientRect(hwnd, &rect);
                    DrawText(hdc,
                            "Hello Windows" ,
                            -1, &rect,
                            DT_SINGLELINE|DT_CENTER|DT_VCENTER);
                    EndPaint(hwnd, &ps);
                    return 0;
                }

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    static LRESULT CALLBACK WndProc(
            HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return ((Window*)GetWindowLongPtr(
                hwnd, GWL_USERDATA))->InstanceProc(hwnd, message, wParam, lParam);
    }
};


#include <d3d10.h>
#include <d3dx10.h>
class DirectX
{
    HWND hwnd_;
    bool initialized_;
    D3D10_DRIVER_TYPE       g_driverType;
    ID3D10Device*           g_pd3dDevice;
    IDXGISwapChain*         g_pSwapChain;
    ID3D10RenderTargetView* g_pRenderTargetView;

public:
    DirectX()
        : hwnd_(0), initialized_(false)
    {
        g_driverType = D3D10_DRIVER_TYPE_NULL;
        g_pd3dDevice = NULL;
        g_pSwapChain = NULL;
        g_pRenderTargetView = NULL;
    }

    void setHwnd(HWND hwnd)
    {
        hwnd_=hwnd;
    }

    void show()
    {
        ShowWindow(hwnd_, SW_SHOWNORMAL);
        UpdateWindow(hwnd_);
    }

    LRESULT InstanceProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch(message)
        {
            case WM_PAINT:
                {
                    if(!initialized_){
                        InitDevice(hwnd);
                    }
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(hwnd, &ps);
                    Render();
                    EndPaint(hwnd, &ps);
                    return 0;
                }

            case WM_ERASEBKGND:
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                return 0;

            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    static LRESULT CALLBACK WndProc(
            HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return ((DirectX*)GetWindowLongPtr(
                    hwnd, GWL_USERDATA))->InstanceProc(hwnd, message, wParam, lParam);
    }

    HRESULT InitDevice(HWND hwnd)
    {
        HRESULT hr = S_OK;;

        RECT rc;
        GetClientRect( hwnd, &rc );
        UINT width = rc.right - rc.left;
        UINT height = rc.bottom - rc.top;

        UINT createDeviceFlags = 0;
#ifdef _DEBUG
        createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

        D3D10_DRIVER_TYPE driverTypes[] =
        {
            D3D10_DRIVER_TYPE_HARDWARE,
            D3D10_DRIVER_TYPE_REFERENCE,
        };
        UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory( &sd, sizeof( sd ) );
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
        {
            g_driverType = driverTypes[driverTypeIndex];
            hr = D3D10CreateDeviceAndSwapChain( NULL, g_driverType, NULL, createDeviceFlags,
                    D3D10_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice );
            if( SUCCEEDED( hr ) )
                break;
        }
        if( FAILED( hr ) )
            return hr;

        // Create a render target view
        ID3D10Texture2D* pBackBuffer;
        hr = g_pSwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBackBuffer );
        if( FAILED( hr ) )
            return hr;

        hr = g_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &g_pRenderTargetView );
        pBackBuffer->Release();
        if( FAILED( hr ) )
            return hr;

        g_pd3dDevice->OMSetRenderTargets( 1, &g_pRenderTargetView, NULL );

        // Setup the viewport
        D3D10_VIEWPORT vp;
        vp.Width = width;
        vp.Height = height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        g_pd3dDevice->RSSetViewports( 1, &vp );

        return S_OK;
    }

    void Render()
    {
        // Just clear the backbuffer
        float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
        g_pd3dDevice->ClearRenderTargetView( g_pRenderTargetView, ClearColor );
        g_pSwapChain->Present( 0, 0 );
    }
};


template<class WINDOW>
std::shared_ptr<WINDOW> registerAndCreate(
        const std::string &className, const std::string &title)
{
    if(!register_class<WINDOW>(className)){
        return std::shared_ptr<WINDOW>();
    }
    auto window=std::shared_ptr<WINDOW>(new WINDOW);
    HWND hwnd=WindowsAPI::create(window.get(), className, title);
    if(!hwnd){
        return std::shared_ptr<WINDOW>();
    }
    window->setHwnd(hwnd);
    return window;
}


int WINAPI WinMain(
        HINSTANCE hInstance ,
        HINSTANCE hPrevInstance ,
        LPSTR lpCmdLine ,
        int nCmdShow ) {

    auto window=registerAndCreate<DirectX>("ClassName", "Hellow Window");
    if(!window){
        return 1;
    }
    window->show();
    return WindowsAPI::loop();
}

