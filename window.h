#ifndef WINDOW_H
#define WINDOW_H

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
struct SimpleVertex
{
        D3DXVECTOR3 Pos;
};


class DirectX
{
    HWND hwnd_;
    bool initialized_;
    D3D10_DRIVER_TYPE       driverType_;
    ID3D10Device*           pd3dDevice_;
    IDXGISwapChain*         pSwapChain_;
    ID3D10RenderTargetView* pRenderTargetView_;

    ID3D10Effect*           pEffect_;
    ID3D10EffectTechnique*  pTechnique_;
    ID3D10InputLayout*      pVertexLayout_;
    ID3D10Buffer*           pVertexBuffer_;
public:
    DirectX()
        : hwnd_(0), initialized_(false),
        driverType_(D3D10_DRIVER_TYPE_NULL),
        pd3dDevice_(NULL),
        pSwapChain_(NULL),
        pRenderTargetView_(NULL),

        pEffect_(NULL),
        pTechnique_(NULL),
        pVertexLayout_(NULL),
        pVertexBuffer_(NULL)
    {
    }

    ~DirectX()
    {
        if( pd3dDevice_ ) pd3dDevice_->ClearState();

        if( pVertexBuffer_ ) pVertexBuffer_->Release();
        if( pVertexLayout_ ) pVertexLayout_->Release();
        if( pEffect_ ) pEffect_->Release();

        if( pRenderTargetView_ ) pRenderTargetView_->Release();
        if( pSwapChain_ ) pSwapChain_->Release();
        if( pd3dDevice_ ) pd3dDevice_->Release();
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
                        if(InitDevice()==S_OK){
                            initialized_=true;
                        }
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

    HRESULT InitDevice()
    {
        HRESULT hr = S_OK;

        RECT rc;
        GetClientRect( hwnd_, &rc );
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
        sd.OutputWindow = hwnd_;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
        {
            driverType_ = driverTypes[driverTypeIndex];
            hr = D3D10CreateDeviceAndSwapChain( NULL, driverType_, NULL, createDeviceFlags,
                    D3D10_SDK_VERSION, &sd, &pSwapChain_, &pd3dDevice_ );
            if( SUCCEEDED( hr ) )
                break;
        }
        if( FAILED( hr ) )
            return hr;

        // Create a render target view
        ID3D10Texture2D* pBuffer;
        hr = pSwapChain_->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBuffer );
        if( FAILED( hr ) )
            return hr;

        hr = pd3dDevice_->CreateRenderTargetView( pBuffer, NULL, &pRenderTargetView_ );
        pBuffer->Release();
        if( FAILED( hr ) )
            return hr;

        pd3dDevice_->OMSetRenderTargets( 1, &pRenderTargetView_, NULL );

        // Setup the viewport
        D3D10_VIEWPORT vp;
        vp.Width = width;
        vp.Height = height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        pd3dDevice_->RSSetViewports( 1, &vp );

        // Create the effect
        DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
        // Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
        // Setting this flag improves the shader debugging experience, but still allows 
        // the shaders to be optimized and to run exactly the way they will run in 
        // the release configuration of this program.
        dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif
        hr = D3DX10CreateEffectFromFile( "Tutorial02.fx", NULL, NULL, "fx_4_0", dwShaderFlags, 0,
                pd3dDevice_, NULL, NULL, &pEffect_, NULL, NULL );
        if( FAILED( hr ) )
        {
            MessageBox( NULL,
                    "The FX file cannot be located.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK );
            return hr;
        }

        // Obtain the technique
        pTechnique_ = pEffect_->GetTechniqueByName( "Render" );

        // Define the input layout
        D3D10_INPUT_ELEMENT_DESC layout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
        };
        UINT numElements = sizeof( layout ) / sizeof( layout[0] );

        // Create the input layout
        D3D10_PASS_DESC PassDesc;
        pTechnique_->GetPassByIndex( 0 )->GetDesc( &PassDesc );
        hr = pd3dDevice_->CreateInputLayout( layout, numElements, PassDesc.pIAInputSignature,
                PassDesc.IAInputSignatureSize, &pVertexLayout_ );
        if( FAILED( hr ) )
            return hr;

        // Set the input layout
        pd3dDevice_->IASetInputLayout( pVertexLayout_ );

        // Create vertex buffer
        SimpleVertex vertices[] =
        {
            D3DXVECTOR3( 0.0f, 0.5f, 0.5f ),
            D3DXVECTOR3( 0.5f, -0.5f, 0.5f ),
            D3DXVECTOR3( -0.5f, -0.5f, 0.5f ),
        };
        D3D10_BUFFER_DESC bd;
        bd.Usage = D3D10_USAGE_DEFAULT;
        bd.ByteWidth = sizeof( SimpleVertex ) * 3;
        bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0;
        bd.MiscFlags = 0;
        D3D10_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = vertices;
        hr = pd3dDevice_->CreateBuffer( &bd, &InitData, &pVertexBuffer_ );
        if( FAILED( hr ) )
            return hr;

        // Set vertex buffer
        UINT stride = sizeof( SimpleVertex );
        UINT offset = 0;
        pd3dDevice_->IASetVertexBuffers( 0, 1, &pVertexBuffer_, &stride, &offset );

        // Set primitive topology
        pd3dDevice_->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

        return S_OK;
    }

    /*
    // Tutorial01
    void Render()
    {
        // Just clear the backbuffer
        float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
        pd3dDevice_->ClearRenderTargetView( pRenderTargetView_, ClearColor );
        pSwapChain_->Present( 0, 0 );
    }
    */

    void Render()
    {
        // Clear the back buffer 
        float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
        pd3dDevice_->ClearRenderTargetView( pRenderTargetView_, ClearColor );

        // Render a triangle
        D3D10_TECHNIQUE_DESC techDesc;
        pTechnique_->GetDesc( &techDesc );
        for( UINT p = 0; p < techDesc.Passes; ++p )
        {
            pTechnique_->GetPassByIndex( p )->Apply( 0 );
            pd3dDevice_->Draw( 3, 0 );
        }

        // Present the information rendered to the back buffer to the front buffer 
        // (the screen)
        pSwapChain_->Present( 0, 0 );
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

#endif
