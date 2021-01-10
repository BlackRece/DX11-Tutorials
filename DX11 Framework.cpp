#include "Application.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

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
            break;
        }
    }

	delete theApp;
	theApp = nullptr;

    return (int) msg.wParam;
}