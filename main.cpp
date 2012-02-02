#include <windows.h>
#include <assert.h>
#include <memory>
#include <string>


class WindowsAPI
{
public:
    WindowsAPI()
    {
    }

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

    template<class WINDOW>
    HWND create(WINDOW *window, const std::string &title, const std::string &className)
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
        

int WINAPI WinMain(
        HINSTANCE hInstance ,
        HINSTANCE hPrevInstance ,
        LPSTR lpCmdLine ,
        int nCmdShow ) {

    auto CLASS_NAME="HelloWindow";

    WindowsAPI api;
    if(!api.register_class<Window>(CLASS_NAME)){
        return 1;
    }
    std::shared_ptr<Window> window(new Window);
    HWND hwnd=api.create(window.get(), "Hello Window", CLASS_NAME);
    if(!hwnd){
        return 2;
    }
    window->setHwnd(hwnd);
    window->show();
    return WindowsAPI::loop();
}

