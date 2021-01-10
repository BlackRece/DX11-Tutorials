#include "Application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    vector<char> rawBuffer;

	Application * theApp = new Application();

	if (FAILED(theApp->Initialise(hInstance, nCmdShow)))
	{
		return -1;
	}

    // Main message loop
    MSG msg = {0};

    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
			theApp->Update();
            theApp->Draw();
        }

        switch (msg.message) {
        case WM_KEYDOWN:
            theApp->OnKeyDown(msg);
            break;

        case WM_KEYUP:
            theApp->OnKeyUp(msg);
            break;

        case WM_INPUT:
            UINT size;

            if (GetRawInputData(
                reinterpret_cast<HRAWINPUT>(msg.lParam),
                RID_INPUT,
                nullptr,
                &size,
                sizeof(RAWINPUTHEADER)) == -1) {
                // bail msg processing if error
                break;
            }

            rawBuffer.resize(size);

            if (GetRawInputData(
                reinterpret_cast<HRAWINPUT>(msg.lParam),
                RID_INPUT,
                rawBuffer.data(),
                &size,
                sizeof(RAWINPUTHEADER)) != size) {
                // bail msg processing if error
                break;
            }

            auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
            if (ri.header.dwType == RIM_TYPEMOUSE &&
                (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0)) {
                theApp->OnMouse(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
            }
            break;
        }
    }

	delete theApp;
	theApp = nullptr;

    return (int) msg.wParam;
}