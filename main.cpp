#include "window.h"


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

